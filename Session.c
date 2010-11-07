/*------------------------------------------------------------------------------
-- SOURCE FILE:     Session.c   - Contains all the OSI "session layer"
--                                functions for the Terminal Emulator.
--
-- PROGRAM:     Advanced Terminal Emulator Pro
--
-- FUNCTIONS:
--              BOOL    Connect(HWND);
--              VOID    Disconnect(HWND);
--              VOID    SelectPort(HWND, INT);
--
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- NOTES:
-- Contains session level functions for the Terminal Emulator program. These
-- are the functions that deal with setting up connections and configuring the
-- port.
------------------------------------------------------------------------------*/

#include "Session.h"

/*------------------------------------------------------------------------------
-- FUNCTION:    Connect
--
-- DATE:        Oct 16, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   Connect(HWND hWnd)
--                          hWnd - the handle to the window
--
-- RETURNS:     True if the serial connection was succefully made.
--
-- NOTES:
--              Opens a serial port connection, displaying appropriate dialogue
--              messages for failed connections. If successful, it sets comm 
--              settings and creates a read thread. It then enables/disables
--              the appropriate menu choices.
------------------------------------------------------------------------------*/
BOOL Connect(HWND hWnd) {
    
    PWNDDATA        pwd         = {0};
    COMMTIMEOUTS    timeOut     = {0};
    DWORD           dwThreadid  = 0;
    DWORD           i           = 0;
	
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    // open serial port
    pwd->hPort = CreateFile(pwd->lpszCommName,
                          GENERIC_READ | GENERIC_WRITE, 0,
                          NULL, OPEN_EXISTING,
                          FILE_FLAG_OVERLAPPED, NULL);

    if (pwd->hPort == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            DISPLAY_ERROR("Serial port does not exist");
        } else {
            DISPLAY_ERROR("Error opening port");
        }
        return FALSE;
    }
    pwd->bConnected = TRUE;


    if (!EscapeCommFunction(pwd->hPort, SETRTS)) {
        DISPLAY_ERROR("Error sending RTS signal");
    }
    if (!EscapeCommFunction(pwd->hPort, SETDTR)) {
        DISPLAY_ERROR("Error sending DTR signal");
    }

    // set timeouts for the port
    if (!GetCommTimeouts(pwd->hPort, &pwd->defaultTimeOuts)) {
        DISPLAY_ERROR("Error retrieving comm timeouts");
        return FALSE;   
    }
    timeOut.ReadIntervalTimeout         = 10;
    timeOut.WriteTotalTimeoutConstant   = 5000;

    if (!SetCommTimeouts(pwd->hPort, &timeOut)) {
        DISPLAY_ERROR("Could not set comm timeouts");
        return FALSE;
    }
    
    // create thread for reading
    pwd->hThread = CreateThread(NULL, 0,
                                (LPTHREAD_START_ROUTINE) ReadThreadProc,
                                hWnd, 0, &dwThreadid);

    if (pwd->hThread == INVALID_HANDLE_VALUE) {
        DISPLAY_ERROR("Error creating read thread");
        return FALSE;
    }
	
	//request packet
	//RequestPacket(hWnd);

    CUR_FG_COLOR = 7;
    CUR_BG_COLOR = 0;
    CUR_STYLE    = 0;
	BRIGHTNESS	 = 0;
	pwd->cursorMode = TRUE;
                                
    // enable/disable appropriate menu choices
    EnableMenuItem(GetMenu(hWnd), IDM_DISCONNECT, MF_ENABLED);
    EnableMenuItem(GetMenu(hWnd), IDM_CONNECT,    MF_GRAYED);
    EnableMenuItem(GetMenu(hWnd), IDM_COMMSET,    MF_GRAYED);
    for (i = 0; i < NO_OF_PORTS; i++) {
        EnableMenuItem(GetMenu(hWnd), IDM_COM1 + i, MF_GRAYED);
    }
	
    return TRUE;
}

/*------------------------------------------------------------------------------
-- FUNCTION:    Disconnect
--
-- DATE:        Oct 16, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   Disconnect(HWND hWnd)
--                          hWnd - the handle to the window
--
-- RETURNS:     VOID.
--
-- NOTES:
--              Closes the active serial port and read thread, then
--              enables/disables the appropriate menu choices.
------------------------------------------------------------------------------*/
VOID Disconnect(HWND hWnd) {

    PWNDDATA        pwd         = (PWNDDATA) GetWindowLongPtr(hWnd, 0);
    COMMTIMEOUTS    timeOut     = {0};
    DWORD           dwThreadid  = 0;
    DWORD           i           = 0;
    
    if (pwd->hPort == NULL) {
        return;
    }

    // this will end the outer while loop in the read thread
    pwd->bConnected = FALSE;
   
    if (!SetCommTimeouts(pwd->hPort, &pwd->defaultTimeOuts)) {
        DISPLAY_ERROR("Could not reset comm timeouts to defaults");
    } else {

    // let the read thread finish up
    do {
        GetExitCodeThread(pwd->hThread, &dwThreadid);
    } while (dwThreadid == STILL_ACTIVE);
	}
    CloseHandle(pwd->hThread);
    CloseHandle(pwd->hPort);
    pwd->hPort = NULL;
	
    // enable/disable appropriate menu choices    
    EnableMenuItem(GetMenu(hWnd), IDM_DISCONNECT, MF_GRAYED);
    EnableMenuItem(GetMenu(hWnd), IDM_CONNECT,    MF_ENABLED);
    EnableMenuItem(GetMenu(hWnd), IDM_COMMSET,    MF_ENABLED);
    for (i = 0; i < NO_OF_PORTS; i++) {
        EnableMenuItem(GetMenu(hWnd), IDM_COM1 + i, MF_ENABLED);
    }
	
}

/*------------------------------------------------------------------------------
-- FUNCTION:    SelectPort
--
-- DATE:        Oct 03, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   VOID SelectPort(HWND hWnd, INT iSelected)
--                          hWnd        - the handle to the window
--                          iSelected   - the com port that was selected in the
--                                        menu
--
-- RETURNS:     VOID.
--
-- NOTES:
--              Selects a new port. It changes the menu checkmark to the new
--              port, and it changes the actual port by renaming lpszCommName.
                The INT argument is the menu item that was selected.
------------------------------------------------------------------------------*/
VOID SelectPort(HWND hWnd, INT iSelected) {
    
    PWNDDATA pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    INT prevPortNo = pwd->lpszCommName[3];
    CheckMenuItem(GetMenu(hWnd),
                 (IDM_COM1 - 1) + (prevPortNo - ASCII_DIGIT_OFFSET),
                 MF_UNCHECKED);
    CheckMenuItem(GetMenu(hWnd), iSelected, MF_CHECKED);

    switch (iSelected) {

        case IDM_COM1:  pwd->lpszCommName = TEXT("COM1");   return;
        case IDM_COM2:  pwd->lpszCommName = TEXT("COM2");   return;       
        case IDM_COM3:  pwd->lpszCommName = TEXT("COM3");   return;       
        case IDM_COM4:  pwd->lpszCommName = TEXT("COM4");   return;        
        case IDM_COM5:  pwd->lpszCommName = TEXT("COM5");   return;        
        case IDM_COM6:  pwd->lpszCommName = TEXT("COM6");   return;        
        case IDM_COM7:  pwd->lpszCommName = TEXT("COM7");   return;        
        case IDM_COM8:  pwd->lpszCommName = TEXT("COM8");   return;        
        case IDM_COM9:  pwd->lpszCommName = TEXT("COM9");   return;
    }
}


VOID InitRfid(HWND hWnd){
	PWNDDATA pwd;
	CHAR        psWriteBuf[26]   = {0x30, 0x31, 0x30, 0x41, 0x30, 0x30, 0x30, 0x33, 0x30, 0x31, 0x34,
									0x33, 0x30, 0x36, 0x30, 0x30, 0x01, 0x0A, 0x00, 0x03, 0x01, 0x43,
									0x06, 0x00, 0x4C, 0xB3};
    OVERLAPPED  overlap         = {0};
    DWORD       dwBytesRead     = 0;
    UINT        bufLength       = 26;
	pwd = (PWNDDATA)GetWindowLongPtr(hWnd, 0);



	psWriteBuf[0] = 0x30;
	psWriteBuf[1] = 0x31;
	psWriteBuf[2] = 0x30;
	psWriteBuf[3] = 0x41;
	psWriteBuf[4] = 0x30;
	psWriteBuf[5] = 0x30;
	psWriteBuf[6] = 0x30;
	psWriteBuf[7] = 0x33;
	psWriteBuf[8] = 0x30;
	psWriteBuf[9] = 0x31;
	psWriteBuf[10] = 0x34;
	psWriteBuf[11] = 0x33;
	psWriteBuf[12] = 0x30;
	psWriteBuf[13] = 0x36;
	psWriteBuf[14] = 0x30;
	psWriteBuf[15] = 0x30;
	psWriteBuf[16] = 0x01;
	psWriteBuf[17] = 0x0A;
	psWriteBuf[18] = 0x00;
	psWriteBuf[19] = 0x03;
	psWriteBuf[20] = 0x01;
	psWriteBuf[21] = 0x43;
	psWriteBuf[22] = 0x06;
	psWriteBuf[23] = 0x00;
	psWriteBuf[24] = 0x4C;
	psWriteBuf[25] = 0xB3;
	if (!WriteFile(pwd->hPort, psWriteBuf, bufLength, &dwBytesRead, &overlap)) {
        if (GetLastError() != ERROR_IO_PENDING) {
            return FALSE;
        }
    }
}