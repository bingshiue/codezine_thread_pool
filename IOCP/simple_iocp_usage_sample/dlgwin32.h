#pragma once


#include <windows.h>
#include <windowsx.h>


///////////////////////////////////////////////////////////////////////////////


#define BUFFER_SIZE		(100 * 1024)   // 100kb
#define COMPKEY_EXIT  (1)

typedef struct _READ_CONTEXT {

	HANDLE		hFile;
	DWORD		cbTotalBytesRead;
	DWORD		cbFileSize;

	BYTE		pBuffer [BUFFER_SIZE];

	HANDLE		hSaveFile;
	
	// Worker Thread Info	
	HANDLE		hThread;
	UINT		uThreadId;
	
	// OVERLAPPED
	OVERLAPPED	ol;
	
} READ_CONTEXT, *PREAD_CONTEXT;


///////////////////////////////////////////////////////////////////////////////


#define HANDLE_DLG_MSG(hwnd, msg, fn) \
    case(msg): return SetDlgMsgResult(hwnd, msg, HANDLE_##msg(hwnd, wParam, lParam, fn))


///////////////////////////////////////////////////////////////////////////////


BOOL CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


BOOL OnInitDialog(HWND hWnd, HWND hWndFocus, LPARAM lParam);
void OnCommand(HWND hWnd, int nID, HWND hWndCtl, UINT codeNotify);

unsigned int __stdcall WorkerThreadFunc(PVOID pv);
VOID CleanupContext ( PREAD_CONTEXT pContext );
VOID BeginAsyncRead ( HWND hwnd, LPTSTR pszPathFrom, LPTSTR pszPathTo );
