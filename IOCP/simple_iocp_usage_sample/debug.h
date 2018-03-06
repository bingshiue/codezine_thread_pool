#pragma once

#include <windows.h>

VOID DebugPrint (LPTSTR   szFormat, ... );
void DebugHexDump (DWORD length, PBYTE buffer);


