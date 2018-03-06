#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdarg.h>

#include "debug.h"


#define BUFF_SIZE  (511)


///////////////////////////////////////////////////////////////////////////////


VOID DebugPrint (LPTSTR   szFormat, ... ) {

	TCHAR    szBuffer[BUFF_SIZE + 1];
	INT     nWritten;
	va_list args;

	::ZeroMemory(szBuffer, sizeof(szBuffer));

	// Format error message like printf()
	
	va_start( args, szFormat );

	nWritten = _vsntprintf( szBuffer, BUFF_SIZE, szFormat, args );

	va_end( args );

	// Output debug string

	::OutputDebugString( szBuffer );

	
}


///////////////////////////////////////////////////////////////////////////////



void DebugHexDump (DWORD length, PBYTE buffer) {
    
    DWORD i,count,index;
    CHAR rgbDigits[]="0123456789abcdef";
    CHAR rgbLine[100];
    char cbLine;

    for(index = 0; length; length -= count, buffer += count, index += count) {
    	count = (length > 16) ? 16:length;

	sprintf(rgbLine, "%4.4x  ",index);
	cbLine = 6;

	for(i=0;i<count;i++) {
		rgbLine[cbLine++] = rgbDigits[buffer[i] >> 4];
		rgbLine[cbLine++] = rgbDigits[buffer[i] & 0x0f];
		if(i == 7) {
			rgbLine[cbLine++] = ':';
		} 
		else {
			rgbLine[cbLine++] = ' ';
		}
	}

	for(; i < 16; i++) {
		rgbLine[cbLine++] = ' ';
		rgbLine[cbLine++] = ' ';
		rgbLine[cbLine++] = ' ';
	}

	rgbLine[cbLine++] = ' ';

	for(i = 0; i < count; i++) {
		if(buffer[i] < 32 || buffer[i] > 126) {
			rgbLine[cbLine++] = '.';
		} 
		else {
			rgbLine[cbLine++] = buffer[i];
		}
	}

        rgbLine[cbLine++] = 0;

	 TCHAR szDebug [128];
	 wsprintf (szDebug, TEXT("%s\n"), rgbLine);
	 OutputDebugString (szDebug);

    }
    

} // end PrintHexDump

