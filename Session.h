#ifndef SESSION_H
#define SESSION_H

#include "Main.h"

BOOL    Connect(HWND hWnd);
VOID    Disconnect(HWND hWnd);
VOID    SelectPort(HWND hWnd, INT iSelected);
VOID	InitRfid(HWND hWnd);

#endif
