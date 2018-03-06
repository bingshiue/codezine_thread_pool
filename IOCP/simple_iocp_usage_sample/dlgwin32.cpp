
#include "dlgwin32.h"
#include "debug.h"
#include "resource.h"

#include <process.h>
#include <assert.h>


#define NUMBER_OF_THREAD (5)

HANDLE g_hIOCP = NULL;


///////////////////////////////////////////////////////////////////////////////


int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow ) {

	//
	// Create I/O Completion Port
	//

	g_hIOCP = CreateIoCompletionPort (
		INVALID_HANDLE_VALUE,
		NULL,
		NULL,
		NUMBER_OF_THREAD - 2);

	if ( !g_hIOCP ) {
		::MessageBox ( NULL, TEXT("CreateIoCompletionPort failed."), TEXT("Error"), MB_OK | MB_ICONERROR );
		return FALSE;
	}

	//
	// Create Worker thread pool
	//

	HANDLE hThreads [NUMBER_OF_THREAD];
	
	for ( int i=0; i < NUMBER_OF_THREAD; i++ ) {
		
		UINT uThreadId;
		
		hThreads[i] = (HANDLE) _beginthreadex(
			NULL, 
			NULL, 
			WorkerThreadFunc, 
			(void*) NULL, 
			NULL, 
			&uThreadId);

		if ( !hThreads[i] ) {
			::MessageBox ( NULL, TEXT("Failed to begin a new thread."), TEXT("Error"), MB_OK | MB_ICONERROR );
			return FALSE;
		}
	}

 	::DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAINWND), NULL, DialogProc );

	//
	// Terminate all the worker threads
	//
	
	for ( int i=0; i<NUMBER_OF_THREAD; i++ ) {

		LPOVERLAPPED pOl = (LPOVERLAPPED) malloc ( sizeof (OVERLAPPED) );
		if ( !pOl ) {
			DebugPrint ( TEXT("Failed to allocate OVERLAPPED. \n") );
			continue;
		}
		
		PostQueuedCompletionStatus ( g_hIOCP, 0, COMPKEY_EXIT, pOl);
		
	}

	//
	// Wait for exiting all the worker threads.
	// Timeout - 30 sec.
	//

	WaitForMultipleObjects ( NUMBER_OF_THREAD, hThreads, TRUE, 30 * 1000 );

	return 0;
}


///////////////////////////////////////////////////////////////////////////////


BOOL CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch(uMsg) {
	HANDLE_DLG_MSG (hwnd, WM_INITDIALOG, OnInitDialog);
	HANDLE_DLG_MSG (hwnd, WM_COMMAND, OnCommand);
	}

	return FALSE;
}


///////////////////////////////////////////////////////////////////////////////


BOOL OnInitDialog(HWND hwnd, HWND hWndFocus, LPARAM lParam) {

	SetDlgItemText ( hwnd, IDC_PATH, TEXT("C:\\foo.dat") );
	
	return TRUE;

}


///////////////////////////////////////////////////////////////////////////////


void OnCommand(HWND hwnd, int nID, HWND hWndCtl, UINT codeNotify) {

	switch(nID) {
	case IDC_READ: {

		TCHAR szPathFrom [ MAX_PATH ];
		Edit_GetText(GetDlgItem(hwnd, IDC_PATH), szPathFrom, sizeof (szPathFrom) );

		TCHAR szPathTo [MAX_PATH];
		if (!GetTempFileName ( TEXT("C:\\temp"), TEXT("adf"), 0, szPathTo ) ) {
			::MessageBox ( hwnd, TEXT("GetTempFileName failed."), TEXT("Error"), MB_OK | MB_ICONERROR );
			break;
		}

		SetDlgItemText ( hwnd, IDC_TO_FILE, szPathTo );

		BeginAsyncRead ( hwnd, szPathFrom, szPathTo );

		break;
	}
	case IDCANCEL:
		EndDialog(hwnd, 0);
		break;

	}
}


///////////////////////////////////////////////////////////////////////////////


VOID BeginAsyncRead ( HWND hwnd, LPTSTR pszPathFrom, LPTSTR pszPathTo ) {

	//
	// Open the file (FROM)
	//
	
	HANDLE hFile = CreateFile ( 
		pszPathFrom, 
		GENERIC_READ,
		NULL,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		NULL);

	if ( INVALID_HANDLE_VALUE == hFile ) {
		::MessageBox ( hwnd, TEXT("CreateFile failed - hFile."), TEXT("Error"), MB_OK | MB_ICONERROR);
		return;
	}

	//
	// Open the file (TO)
	//

	HANDLE hSaveFile = CreateFile (
		pszPathTo,
		GENERIC_WRITE,
		NULL,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if ( INVALID_HANDLE_VALUE == hSaveFile ) {
		::MessageBox ( hwnd, TEXT("CreateFile failed - hSaveFile."), TEXT("Error"), MB_OK | MB_ICONERROR);
		CloseHandle ( hFile );
		return;
	}

	//
	// Associate the file handle with I/O Completion Port.
	//

	HANDLE hIOCP;

	hIOCP = CreateIoCompletionPort ( hFile, g_hIOCP, NULL, 0);

	if ( !hIOCP ) {
		::MessageBox ( hwnd, TEXT("CreateIoCompletionPort failed."), TEXT("Error"), MB_OK | MB_ICONERROR );
		CloseHandle ( hFile );
		CloseHandle ( hSaveFile );
		return;
	}
	
	//
	// Create a CONTEXT
	//
	
	PREAD_CONTEXT pContext = (PREAD_CONTEXT) malloc ( sizeof (READ_CONTEXT) );
	DWORD cbFileSize = GetFileSize ( hFile, NULL );
	
	if ( !pContext || INVALID_FILE_SIZE == cbFileSize ) {
		::MessageBox ( hwnd, TEXT("Failed to allocate the memory or get the file size."), TEXT("Error"), MB_OK | MB_ICONERROR);
		CloseHandle ( hFile );
		CloseHandle ( hSaveFile );
		return;
	}
	ZeroMemory ( pContext , sizeof (READ_CONTEXT) );

	pContext->hFile = hFile;
	pContext->cbTotalBytesRead = 0;
	pContext->cbFileSize = cbFileSize;
	pContext->ol.Offset = 0;
	pContext->hSaveFile = hSaveFile;
	
	//
	// Read file
	//

	BOOL bRet = ReadFile (
		hFile,
		pContext->pBuffer,
		BUFFER_SIZE,
		NULL,
		&pContext->ol );
		
	// Completed?
	if ( !bRet ) {
		DWORD dwGle = GetLastError ();

		if ( ERROR_IO_PENDING != dwGle ) {
			// Error
			TCHAR szErrorMessage [256];
			wsprintf ( szErrorMessage, TEXT("ReadFile failed (%u)"), dwGle );
			::MessageBox ( hwnd, szErrorMessage, TEXT("Error"), MB_OK | MB_ICONERROR );
			// Cleanup
			CleanupContext ( pContext );
		}
	}
	

}


///////////////////////////////////////////////////////////////////////////////


VOID CleanupContext ( PREAD_CONTEXT pContext ) {

	if ( !pContext ) {
		return;
	}

	if ( pContext->hFile ) {
		CloseHandle ( pContext->hFile );
		pContext->hFile = NULL;
	}
	
	if ( pContext->hSaveFile ) {
		CloseHandle ( pContext->hSaveFile );
		pContext->hSaveFile = NULL;
	}

	free ( pContext );

}


///////////////////////////////////////////////////////////////////////////////


unsigned int __stdcall WorkerThreadFunc (PVOID pv) {

	DebugPrint ( TEXT("WorkerThreadFunc %08x Start\n"), GetCurrentThreadId() );

	PREAD_CONTEXT pContext;
	LPOVERLAPPED pol = NULL;
	DWORD cbNumberOfBytesTransferred = 0;
	ULONG uCompletionKey;
	BOOL bRet;
	
	while (1) {

		bRet = GetQueuedCompletionStatus (
			g_hIOCP, 
			&cbNumberOfBytesTransferred, 
			&uCompletionKey, 
			&pol, 
			INFINITE );
		
		if ( !bRet ) {
			// Error!
			assert ( FALSE );
			
			break;
		}

		//
		// Exit thread
		//
		
		if ( COMPKEY_EXIT == uCompletionKey ) {
			free ( pol );
			break;
		}

		//
		// Retreive a pointer to PREAD_CONTEXT with pol
		//
		
		pContext = (PREAD_CONTEXT) CONTAINING_RECORD (pol, READ_CONTEXT, ol);

		//
		// Update the context
		//

		pContext->cbTotalBytesRead += cbNumberOfBytesTransferred;
		pContext->ol.Offset += cbNumberOfBytesTransferred;

		DebugPrint ("TID:%p - TotalBytesRead = %u", GetCurrentThreadId(), pContext->cbTotalBytesRead );
		
		//
		// Use the buffer
		//
		DWORD cbNumberOfBytesWritten = 0;
		BOOL bWrite;
		bWrite = WriteFile ( 
			pContext->hSaveFile, 
			pContext->pBuffer,
			cbNumberOfBytesTransferred,
			&cbNumberOfBytesWritten,
			NULL );

		if ( !bWrite ) {
			DebugPrint ( TEXT("WriteFile failed %u\n"), GetLastError() );
			CleanupContext ( pContext );
			continue;
		}
		
		// DebugHexDump ( cbNumberOfBytesTransferred, (PBYTE) pContext->pBuffer );
		ZeroMemory ( pContext->pBuffer, BUFFER_SIZE );

		//
		// Need to read again?
		//

		if ( pContext->cbFileSize == pContext->cbTotalBytesRead ) {

			//
			// Read and save the file completely.
			// Clean up the context
			//

			CleanupContext ( pContext );

			DebugPrint ( TEXT("OK!!\n") );
			
			continue;
		}
		
		//
		// Call ReadFile again.
		//

		bRet = ReadFile (
			pContext->hFile,
			pContext->pBuffer,
			min (BUFFER_SIZE, (pContext->cbFileSize - pContext->cbTotalBytesRead)),
			NULL,
			&pContext->ol );

		if ( !bRet ) {

			DWORD dwGle = GetLastError ();

			if ( ERROR_IO_PENDING != dwGle ) {

				CleanupContext ( pContext );
				DebugPrint ( TEXT("ERROR!! %u\n"), dwGle );
				
			}
		}
		
	}

	DebugPrint ( TEXT("WorkerThreadFunc %08x End\n"), GetCurrentThreadId() );

	return 0;
}

