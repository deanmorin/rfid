#ifndef ERRORDETECT_H
#define ERRORDETECT_H

#include "TerminalEmulator.h"
#include <stdio.h>
BOOL DetectLRCError(CHAR* pcPacket, DWORD dwLength);

#endif
