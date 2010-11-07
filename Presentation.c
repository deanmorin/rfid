/*------------------------------------------------------------------------------
-- SOURCE FILE:     Presentation.c - Contains all the OSI "presentation layer"
--                                   functions for the Terminal Emulator.
--
-- PROGRAM:     Advanced Terminal Emulator Pro
--
-- FUNCTIONS:
--              BOOL    ProcessWrite(HWND hWnd, WPARAM wParam, 
--                                   BOOL bNonCharKey);
--              VOID    ProcessRead(HWND hWnd, CHAR psReadBuf[], 
--                                  DWORD dwBytesRead);
--              VOID    ProcessSpecialChar(HWND hWnd, CHAR cSpChar);
--              VOID    UpdateDisplayBuf(HWND hWnd, CHAR cCharacter);
--              VOID    Bell(HWND hWnd);
--              VOID    HorizontalTab(HWND hWnd);
--              VOID    FormFeed(HWND hWnd);
--              VOID    MoveCursor(HWND hWnd, INT cxCoord, INT cyCoord, 
--                                 BOOL bScroll);
--              VOID    ClearLine(HWND hWnd, UINT cxCoord, UINT cyCoord, 
--                                INT iDirection);
--              VOID    ClearScreen(HWND hWnd, UINT cxCoord, UINT cyCoord, 
--                                  INT iDirection);
--              VOID    ScrollDown(HWND hWnd);
--              VOID    ScrollUp(HWND hWnd);
--              VOID    SetScrollRegion(HWND hWnd, INT cyTop, INT cyBottom); 
--				VOID	RequestPacket(HWND hWnd);
--				VOID	ProcessPacket(HWND hWnd, CHAR* pcPacket, DWORD dwLength);
--				VOID	MakeColumns(HWND hWnd);
--				VOID EchoTag(HWND hWnd, CHAR* pcToken, DWORD dwTokenLength, CHAR* pcData, DWORD dwDataLength)
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   November 4, 2010 - Added RequestPacket, ProcessPacket, EchoTag, MakeColumns
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- NOTES:
-- Contains presentation level functions for the Terminal Emulator program.
-- These are the functions that process the characters both typed and received
-- into meaningful data.
------------------------------------------------------------------------------*/

#include "Presentation.h"

/*------------------------------------------------------------------------------
-- FUNCTION:    ProcessWrite
--
-- DATE:        Oct 13, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   BOOL ProcessWrite(HWND hWnd, WPARAM wParam, BOOL bNonCharKey)
--                          hWnd        - the handle to the window
--                          wParam      - the key that was pressed
--                          bNonCharKey - true if the key pressed was not a
--                                        standard displayable character
--
-- RETURNS:     True if the port write was successful.
--
-- NOTES:
--              Writes wParam (the key pressed) to the port, if. If the key is a
--              special non-character key such as 'home' or 'arrow up', it is 
--              first translated to its corresponding command sequence.
------------------------------------------------------------------------------*/
BOOL ProcessWrite(HWND hWnd, WPARAM wParam, BOOL bNonCharKey) {
 
    PWNDDATA    pwd             = {0};
    CHAR        psWriteBuf[4]   = {0};
    OVERLAPPED  overlap         = {0};
    DWORD       dwBytesRead     = 0;
    UINT        bufLength       = 1;
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    if (bNonCharKey) {

        psWriteBuf[0] = VK_ESCAPE;
		if (wParam <= VK_DOWN && pwd->cursorMode) { 
            psWriteBuf[1] = '[';
        } else {
            psWriteBuf[1] = 'O';
        }
        bufLength = 3;

        switch (wParam) {
            case VK_UP:     psWriteBuf[2] = 'A';    break;
            case VK_DOWN:   psWriteBuf[2] = 'B';    break;
            case VK_RIGHT:  psWriteBuf[2] = 'C';    break;
            case VK_LEFT:   psWriteBuf[2] = 'D';    break;
            case VK_HOME:   psWriteBuf[2] = 'H';    break;
            case VK_END:    psWriteBuf[2] = 'K';    break;
            case VK_F1:     psWriteBuf[2] = 'P';    break;
            case VK_F2:     psWriteBuf[2] = 'Q';    break;
            case VK_F3:     psWriteBuf[2] = 'R';    break;
            case VK_F4:     psWriteBuf[2] = 'S';    break;
        }

    } else {
        psWriteBuf[0] = wParam;
    }

    if (!WriteFile(pwd->hPort, psWriteBuf, bufLength, &dwBytesRead, &overlap)) {
        if (GetLastError() != ERROR_IO_PENDING) {
            return FALSE;
        }
    }
    return TRUE;
}
/*------------------------------------------------------------------------------
-- FUNCTION:    RequestPacket
--
-- DATE:        Nov 4, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Daniel Wright
--
-- PROGRAMMER:  Daniel Wright
--
-- INTERFACE:   BOOL RequestPacket(HWND hWnd)
--                          hWnd        - the handle to the window
--                          
-- RETURNS:     True if the port write was successful.
--
-- NOTES:
--              Writes a string representing a packet request to the port.
------------------------------------------------------------------------------*/
BOOL RequestPacket(HWND hWnd) {
 
    PWNDDATA    pwd             = {0};
    CHAR        psWriteBuf[10]   = {0};
    OVERLAPPED  overlap         = {0};
    DWORD       dwBytesRead     = 0;
    UINT        bufLength       = 9;
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    psWriteBuf[0] = 0x01;
	psWriteBuf[1] = 0x09;
	psWriteBuf[2] = 0x00;
	psWriteBuf[3] = 0x03;
	psWriteBuf[4] = 0x01;
	psWriteBuf[5] = 0x41;
	psWriteBuf[6] = 0x00; 
	psWriteBuf[7] = 0x4B; 
	psWriteBuf[8] = 0xB4; 

    if (!WriteFile(pwd->hPort, psWriteBuf, bufLength, &dwBytesRead, &overlap)) {
        if (GetLastError() != ERROR_IO_PENDING) {
            return FALSE;
        }
    }
    return TRUE;
}

/*------------------------------------------------------------------------------
-- FUNCTION:    ProcessRead
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin, Marcel Vangrootheest
--
-- PROGRAMMER:  Dean Morin, Marcel Vangrootheest
--
-- INTERFACE:   VOID ProcessRead(HWND, CHAR*, DWORD)
--
-- RETURNS:     VOID.
--
-- NOTES:
--              All characters read off the serial port are passed to this
--              function, which checks the buffer and passes it to the 
--              appropriate processing function. If an unfinished escape 
--              sequence was previously processed, then the buffer that was 
--              passed in is appended to the incomplete one, and passed to 
--              ProcessEsc().
------------------------------------------------------------------------------*/
VOID ProcessRead(HWND hWnd, CHAR* psReadBuf, DWORD dwBytesRead) {

    PWNDDATA    pwd                 = NULL;
    CHAR*       psCombined          = NULL;
    DWORD       dwCombinedLength    = 0;
    DWORD       i                   = 0;
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);
    
    if (pwd->psIncompleteEsc) {
        // the last escape sequence was incomplete, but still could be valid
        dwCombinedLength    = pwd->dwIncompleteLength + dwBytesRead;
        psCombined          = (CHAR*) malloc(sizeof(CHAR) * dwCombinedLength);
        strncpy(psCombined, pwd->psIncompleteEsc, pwd->dwIncompleteLength);
        strncpy((psCombined + pwd->dwIncompleteLength), psReadBuf, dwBytesRead);
		pwd->psIncompleteEsc = NULL;
        ProcessEsc(hWnd, psCombined, dwCombinedLength);
        return;
    }
    for (i = 0; i < dwBytesRead; i++) {
        if (psReadBuf[i] == VK_ESCAPE) {
            ProcessEsc(hWnd, psReadBuf + i, dwBytesRead - i);
            return;
        }
        if (psReadBuf[i] >= VK_SPACE  &&  psReadBuf[i] <= '~') {
            UpdateDisplayBuf(hWnd, psReadBuf[i]);
        } else {
            ProcessSpecialChar(hWnd, psReadBuf[i]);
        }
    }
}

/*------------------------------------------------------------------------------
-- FUNCTION:    ProcessSpecialChar
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   BOOL ProcessSpecialChar(HWND hWnd, CHAR cSpChar)
--                          hWnd    - the handle to the window
--                          cSpChar - a special character that might need
--                                    processing
--
-- RETURNS:     VOID.
--
-- NOTES:
--              Calls a function to handle the special character, cSpChar.
------------------------------------------------------------------------------*/
VOID ProcessSpecialChar(HWND hWnd, CHAR cSpChar) {
    
    PWNDDATA pwd = NULL;
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    switch (cSpChar) {
        // bell
        case 0x07:  Bell(hWnd);                             break;
        // backspace
        case 0x08:  MoveCursor(hWnd, X, Y + 1, FALSE);      break;
        // horizontal tab
        case 0x09:  HorizontalTab(hWnd);                    break;
        // line feed
        case 0x0A:  MoveCursor(hWnd, 1, Y + 2, TRUE);       break;
        // vertical tab
        case 0x0B:  MoveCursor(hWnd, X + 1, Y + 2, TRUE);   break;
        // form feed
        case 0x0C:  FormFeed(hWnd);                         break;
        // carraige return
        case 0x0D:  MoveCursor(hWnd, 1, Y + 1, FALSE);      break;
    }
}
/*------------------------------------------------------------------------------
-- FUNCTION:    ProcessPacket
--
-- DATE:        Nov 4, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin, Marcel Vangrootheest
--
-- PROGRAMMER:  Daniel Wright
--
-- INTERFACE:   VOID ProcessPacket(HWND hWnd, CHAR* pcPacket, DWORD dwLength)
--                          hWnd    - the handle to the window
--                          pcPacket - RFID packet
--							dwLength - number of bytes in the RFID packet
--
-- RETURNS:     VOID.
--
-- NOTES:
--              Calls DetectLRCError to check for errors in the packet.
--				Calls a function to display token name and data.
------------------------------------------------------------------------------*/
VOID ProcessPacket(HWND hWnd, CHAR* pcPacket, DWORD dwLength){
	PWNDDATA pwd = NULL;
	
	CHAR pcToken[512];
	DWORD dwTokenLength = 0;
	CHAR pcData[512];
	DWORD dwDataLength = 0;
	DWORD j, i;
	pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

	if(DetectLRCError(pcPacket, dwLength)){
		DISPLAY_ERROR("Error in RFID Packet");
	}
	
	
	switch(pcPacket[7]){
		case 0x04:

			strcpy(pcToken , "ISO 15693");
			dwTokenLength = strlen(pcToken);
			dwDataLength = 8;
			j = (dwLength - 3);
			for(i = 0; i < dwDataLength; i++){
				pcData[i] = pcPacket[j];
				j--;
			}
			EchoTag(hWnd, pcToken, dwTokenLength, pcData, dwDataLength);
			return;
		case 0x05:
			
			strcpy(pcToken , "TAG-IT HF");
			dwTokenLength = strlen(pcToken);
			dwDataLength = 4;
			for(i = 0, j = (dwLength - 1); i < dwDataLength; i++, j--){
				pcData[i] = pcPacket[j];
			}
			EchoTag(hWnd, pcToken, dwTokenLength, pcData, dwDataLength);
			
			return;
		case 0x06:
	
			strcpy(pcToken , "LF R/W");
			dwTokenLength = strlen(pcToken);
			dwDataLength = 8;
			for(i = 0, j = (dwLength - 3); i < dwDataLength; i++, j--){
				pcData[i] = pcPacket[j];
			}
			EchoTag(hWnd, pcToken, dwTokenLength, pcData, dwDataLength);
			
			return;
		
			
		default:
			//Ignore response to Rfid initialization
			if(pcPacket[1] == 0x21){
				return;
			}
			strcpy(pcToken, "Unsupported Tag");
			for(i = 0; i < strlen(pcToken); i++){
				UpdateDisplayBuf(hWnd, pcToken[i]);
			}
			return;
	}
}
VOID EchoTag(HWND hWnd, CHAR* pcToken, DWORD dwTokenLength, CHAR* pcData, DWORD dwDataLength){
	DWORD i;
	CHAR* temp = (CHAR*)malloc(sizeof(CHAR)*dwDataLength*2);
    SetScrollRegion(hWnd,2,LINES_PER_SCRN);
	ScrollUp(hWnd);
	MoveCursor( hWnd, 1, 2, FALSE);
	for(i=0;i<dwTokenLength;i++){
		UpdateDisplayBuf(hWnd,pcToken[i]);
	}
    MoveCursor( hWnd, 12, 2, FALSE);
    
	for(i=0;i<dwDataLength;i++)
	  sprintf(temp+2*i, "%02X", (BYTE)pcData[i]);


	for(i=0;i<dwDataLength*2;i++){
      
        UpdateDisplayBuf(hWnd,temp[i++]);
		UpdateDisplayBuf(hWnd,temp[i]);
		UpdateDisplayBuf(hWnd,' ');
	}
    SetScrollRegion(hWnd,1,LINES_PER_SCRN);
}

VOID MakeColumns(HWND hWnd){
    CHAR temp1[10]= "Token";
    CHAR temp2[10]= "Value";
	DWORD i;
    MoveCursor( hWnd, 1, 1, FALSE);
    for(i=0;i<10;i++){
        UpdateDisplayBuf(hWnd,temp1[i]);
    }
    MoveCursor( hWnd, 12, 1, FALSE);
    for(i=0;i<10;i++){
        UpdateDisplayBuf(hWnd,temp2[i]);
    }    

    
}



/*------------------------------------------------------------------------------
-- FUNCTION:    UpdateDisplayBuf
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   VOID UpdateDisplayBuf(HWND hWnd, CHAR cCharacter)
--                          hWnd        - the handle to the window
--                          cCharacter  - the character to add to the buffer
--
-- RETURNS:     VOID.
--
-- NOTES:
--              Adds cCharacter to the display buffer.
------------------------------------------------------------------------------*/
VOID UpdateDisplayBuf(HWND hWnd, CHAR cCharacter) {
    
    PWNDDATA    pwd     = NULL;
    CHAR        a[2]    = {0};    
    HDC         hdc     = {0};
    CHARINFO    ci      = {0};
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    a[0] = cCharacter;
     
    CHARACTER(X, Y).character   = cCharacter;
    CHARACTER(X, Y).fgColor     = CUR_FG_COLOR;
	CHARACTER(X, Y).bgColor     = CUR_BG_COLOR;
	CHARACTER(X, Y).style	    = CUR_STYLE;
    
    if (X >= CHARS_PER_LINE - 1) { 
        if (pwd->wordWrap == FALSE) {
			return;
		}
		X = 0;
		if (Y < LINES_PER_SCRN - 1) { 
            Y++;
        } else {
            ScrollDown(hWnd);
        }
    } else {
        X++;
    }
}

/*------------------------------------------------------------------------------
-- FUNCTION:    Bell
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   VOID Bell(HWND hWnd)
--                          hWnd - the handle to the window
--
-- RETURNS:     VOID.
--
-- NOTES:
--              Processes a bell character based on the value of iBellSetting
--              in the PWNDATA structure. The bell is ignored by default, and
--              can be set to flash, or to play a sound.
------------------------------------------------------------------------------*/
VOID Bell(HWND hWnd) {
    PWNDDATA    pwd     = NULL;
    HDC         hdc     = {0};
    RECT        rect    = {0};
    static TCHAR* beeps[6] = {TEXT("beep1.wav"), TEXT("beep2.wav"), 
                              TEXT("beep3.wav"), TEXT("beep4.wav"),
                              TEXT("beep5.wav"), TEXT("beep6.wav")};
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);
    
    if (pwd->iBellSetting == IDM_BELL_VIS) {
        GetClientRect(hWnd, &rect);
        SetRect (&rect, rect.top, rect.left, rect.right, rect.bottom);
        hdc = GetDC(hWnd);
        FillRect (hdc, &rect, CreateSolidBrush(RGB(255,255,255)));
        ReleaseDC(hWnd, hdc);
        Sleep(30);
        InvalidateRect(hWnd, NULL, TRUE);
 
    } else if (pwd->iBellSetting == IDM_BELL_AUR) {

       PlaySound(beeps[rand() % 6], NULL, SND_FILENAME | SND_ASYNC);

    }
}

/*------------------------------------------------------------------------------
-- FUNCTION:    HorizontalTab
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   VOID HorizontalTab(HWND hWnd)
--                          hWnd - the handle to the window
--
-- RETURNS:     VOID.
--
-- NOTES:
--              Moves the cursor to the next tab on the line.
------------------------------------------------------------------------------*/
VOID HorizontalTab(HWND hWnd) {
    PWNDDATA pwd = NULL;
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);   
    if (X < CHARS_PER_LINE - TAB_LENGTH) {
        X += TAB_LENGTH - X % TAB_LENGTH;
    } else {
        X = CHARS_PER_LINE - 1;
    }
}

/*------------------------------------------------------------------------------
-- FUNCTION:    FormFeed
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   VOID FormFeed(HWND hWnd)
--                          hWnd - the handle to the window
--
-- RETURNS:     VOID.
--
-- NOTES:
--              Clears the screen and moves the cursor to 1, 1 (0, 0 according
--              to the display buffer).
------------------------------------------------------------------------------*/
VOID FormFeed(HWND hWnd) { 
    PWNDDATA    pwd = NULL;
    UINT        i   = 0;
    UINT        j   = 0;
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    for (i = 0; i < LINES_PER_SCRN; i++) {
        for (j = 0; j < CHARS_PER_LINE; j++) {
            CHARACTER(j, i).character   = ' ';
            CHARACTER(j, i).bgColor     = CUR_BG_COLOR;
            CHARACTER(j, i).style       = 0;
         }
    }
    X = 0;
    Y = 0;
}

/*------------------------------------------------------------------------------
-- FUNCTION:    MoveCursor
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   VOID MoveCursor(HWND hWnd, UINT cxCoord, UINT cyCoord, 
--                              BOOL bScroll)
--                          hWnd        - the handle to the window
--                          cxCoord     - the x coordinate to move the cursor to
--                                        (1,1) origin
--                          cyCoord     - the y coordinate to move the cursor to
--                                        (1,1) origin
--                          bScroll     - whether or not to scroll when the
--                                        y coordinate is beyond the bounds of
--                                        screen
--
-- RETURNS:     VOID.
--
-- NOTES:
--              Moves the cursor to the specified position. The position passed 
--              in the arguments is based screen coordinates(1,1) origin).
--              If bScroll is true, then the screen will scroll when the top
--              of bottom lines of the window are reached.
------------------------------------------------------------------------------*/
VOID MoveCursor(HWND hWnd, INT cxCoord, INT cyCoord, BOOL bScroll) {
    PWNDDATA pwd = NULL;
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);
    
    if (cxCoord < 1) {
        X = 0;
    } else if (cxCoord > CHARS_PER_LINE) {
        X = CHARS_PER_LINE - 1;
    } else {
        X = --cxCoord;
    }

    if (cyCoord < WINDOW_TOP + 1) {
        Y = WINDOW_TOP;
        if (bScroll) {
            ScrollUp(hWnd);
        }
    } else if (cyCoord > WINDOW_BOTTOM + 1) {
        Y = WINDOW_BOTTOM;
        if (bScroll) {
            ScrollDown(hWnd);
        }
    } else {
        Y = --cyCoord;
    }
}

/*------------------------------------------------------------------------------
-- FUNCTION:    ClearLine
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   VOID ClearLine(HWND hWnd, UINT cxCoord, UINT cyCoord, 
--                             INT iDirection)
--                          hWnd        - the handle to the window
--                          cxCoord     - the row of the first 
--                                        character to clear - (0,0) origin
--                          cyCoord     - the line of the first
--                                        character to clear - (0,0) origin
--                          iDirection  - the direction (left or right) to clear
--                                        the line
--
-- RETURNS:     VOID.
--
-- NOTES:
--              Clears a line in the direction specified by iDirection (left or
--              right). The character under the cursor will be cleared as well.
--              Please note that cxCoord and cyCoord use a (0,0) origin.
------------------------------------------------------------------------------*/
VOID ClearLine(HWND hWnd, UINT cxCoord, UINT cyCoord, INT iDirection) {
    PWNDDATA    pwd = NULL;
    UINT        i   = 0;
    UINT        j   = 0;
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);
    
    i = cyCoord;
    j = cxCoord;
    while (j < CHARS_PER_LINE  &&  j >= 0) {
        CHARACTER(j, i).character   = ' ';
        CHARACTER(j, i).bgColor     = CUR_BG_COLOR;
        CHARACTER(j, i).style       = 0;
        j += iDirection;
    }
}

/*------------------------------------------------------------------------------
-- FUNCTION:    ClearScreen
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   VOID ClearScreen(HWND hWnd, UINT cxCoord, UINT cyCoord, 
--                               INT iDirection)
--                          hWnd        - the handle to the window
--                          cxCoord     - the row of the first 
--                                        character to clear - (0,0) origin
--                          cyCoord     - the line of the first
--                                        character to clear - (0,0) origin
--                          iDirection  - the direction (up or down) to clear
--                                        the screen
--
-- RETURNS:     VOID.
--
-- NOTES:
--              Clears the screen in the direction specified by iDirection (all
--              preceding characters, or all following characters). The
--              character under the cursor will be cleared as well. Please note
--              that cxCoord and cyCoord use a (0,0) origin.
------------------------------------------------------------------------------*/
VOID ClearScreen(HWND hWnd, UINT cxCoord, UINT cyCoord, INT iDirection) {
    PWNDDATA    pwd = NULL;
    UINT        i   = 0;
    UINT        j   = 0;
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    ClearLine(hWnd, cxCoord, cyCoord, iDirection);

    i = cyCoord + iDirection;
    while (i < LINES_PER_SCRN  &&  i >= 0) {
        for (j = 0; j < CHARS_PER_LINE; j++) {
            CHARACTER(j, i).character   = ' ';
            CHARACTER(j, i).bgColor     = CUR_BG_COLOR;
            CHARACTER(j, i).style       = 0;
         }
         i += iDirection;
    }
}

/*------------------------------------------------------------------------------
-- FUNCTION:    ScrollDown
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   VOID ScrollDown(HWND hWnd)
--                          hWnd - the handle to the window
--
-- RETURNS:     VOID.
--
-- NOTES:
--              "Scrolls down" one line. It moves every line on the screen up
--              one position, deleting the top line, and creating a new, blank
--              bottom line.
------------------------------------------------------------------------------*/
VOID ScrollDown(HWND hWnd) {
    PWNDDATA    pwd         = NULL;
    PLINE       pNewLine    = NULL;
    INT         i           = 0;
    INT         j           = 0;
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0); 
    pNewLine = (PLINE) malloc(sizeof(LINE));
    free(ROW(WINDOW_TOP));

    for (i = WINDOW_TOP; i < WINDOW_BOTTOM; i++) {
        ROW(i) = ROW(i + 1);
    }
    ROW(i) = pNewLine;
    for (j = 0; j < CHARS_PER_LINE; j++) {
        CHARACTER(j, i).character   = ' ';
        CHARACTER(j, i).bgColor     = CUR_BG_COLOR;
        CHARACTER(j, i).style       = 0;
    }
}

/*------------------------------------------------------------------------------
-- FUNCTION:    ScrollUp
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   VOID ScrollUp(HWND hWnd)
--                          hWnd - the handle to the window
--
-- RETURNS:     VOID.
--
-- NOTES:
--              "Scrolls up" one line. It moves every line on the screen down
--              one position, deleting the bottom line, and creating a new, 
--              blank top line.
------------------------------------------------------------------------------*/
VOID ScrollUp(HWND hWnd) {
    PWNDDATA    pwd         = NULL;
    PLINE       pNewLine    = NULL;
    INT         i           = 0;
    INT         j           = 0;
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0); 
    pNewLine = (PLINE) malloc(sizeof(LINE));
    free(ROW(WINDOW_BOTTOM));
    
    for (i = WINDOW_BOTTOM; i > WINDOW_TOP; i--) {
        ROW(i) = ROW(i - 1);
    }
    ROW(i) = pNewLine;
    for (j = 0; j < CHARS_PER_LINE; j++) {
        CHARACTER(j, i).character   = ' ';
        CHARACTER(j, i).bgColor     = CUR_BG_COLOR;
        CHARACTER(j, i).style       = 0;
    }
}

/*------------------------------------------------------------------------------
-- FUNCTION:    SetScrollRegion
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   VOID SetScrollRegion(HWND hWnd, INT cyTop, INT cyBottom)
--                          hWnd     - the handle to the window
--                          cyTop    - the top line of the new scroll range
--                          cyBottom - the bottom line of the new scroll range
--
-- RETURNS:     VOID.
--
-- NOTES:
--              Sets the top and bottom lines for the scrollable region. This
--              allows the appearance of scrolling for one section of lines,
--              whereas the other lines will remain motionless.
------------------------------------------------------------------------------*/
VOID SetScrollRegion(HWND hWnd, INT cyTop, INT cyBottom) {
    PWNDDATA pwd = NULL;
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0); 
    MoveCursor(hWnd, 1, cyTop, FALSE);
    WINDOW_TOP      = --cyTop;
    WINDOW_BOTTOM   = --cyBottom;   
}
/*------------------------------------------------------------------------------
-- FUNCTION:    ScreenAlignment
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Marcel Vangrootheest
--
-- PROGRAMMER:  Marcel Vangrootheest
--
-- INTERFACE:   VOID ScreenAlignment(HWND hWnd)
--                          hWnd - the handle to the window
--
-- RETURNS:     VOID.
--
-- NOTES:
--              Fills the screen with E's and moves the cursor to 1, 1 (0, 0 according
--              to the display buffer).
------------------------------------------------------------------------------*/
VOID ScreenAlignment(HWND hWnd) { 
    PWNDDATA    pwd = NULL;
    UINT        i   = 0;
    UINT        j   = 0;
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    for (i = 0; i < LINES_PER_SCRN; i++) {
        for (j = 0; j < CHARS_PER_LINE; j++) {
            CHARACTER(j, i).character   = 'E';
            CHARACTER(j, i).bgColor     = CUR_BG_COLOR;
            CHARACTER(j, i).style       = 0;
         }
    }
    X = 0;
    Y = 0;
}
