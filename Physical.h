#ifndef PHYSICAL_H
#define PHYSICAL_H

#include "List.h"
#include "TerminalEmulator.h"

#define READ_BUFSIZE    1024
#define WAIT_TIME       100

VOID            ProcessCommError(HANDLE hPort);
DWORD WINAPI    ReadThreadProc(HWND hWnd);

#endif
