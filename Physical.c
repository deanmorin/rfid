/*------------------------------------------------------------------------------
-- SOURCE FILE:     Physical.c - Contains all the OSI "physical layer"
--                               functions for the Intelligent Terminal
--                               Emulator. The definitions for the OSI layers
--                               have been loosened somewhat, since the purpose 
--                               is to organize the functions intuitively, 
--                               rather than pedantically.
--                      
-- PROGRAM:     RFID Reader - Enterprise Edition
--
-- FUNCTIONS:
--              DWORD WINAPI    ReadThreadProc(HWND);
--              VOID            ProcessCommError(HANDLE);
--
--
-- DATE:        Oct 13, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- NOTES:
-- Contains physical level functions for the Intelligent Terminal Emulator.
------------------------------------------------------------------------------*/

#include "Physical.h"

/*------------------------------------------------------------------------------
-- FUNCTION:    ReadThreadProc
--
-- DATE:        Oct 13, 2010
--
-- REVISIONS:   Nov 05, 2010
--              Modified the function to also listen for a "disconnect" event,
--              ond to break in that case.
--              ProcessRead() is now called once a complete packet is confirmed
--              (as opposed to sending the contents of the buffer to 
--              ProcessRead() as soon as they arrive.
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin, Daniel Wright
--
-- INTERFACE:   DWORD WINAPI ReadThreadProc(HWND hWnd)
--                          hWnd - the handle to the window
--
-- RETURNS:     0 because threads are required to return a DWORD.
--
-- NOTES:
--              While connected, this thread will loop and wait for characters
--              to arrive at the port. Once the EV_RXCHAR event is triggered,
--              ReadFile() is called to get however many characters have arrived
--              at the port by that time. This function uses overlapped I/O.
------------------------------------------------------------------------------*/
DWORD WINAPI ReadThreadProc(HWND hWnd) {
    
    PWNDDATA        pwd                     = NULL;
    CHAR            psReadBuf[READ_BUFSIZE] = {0};
    OVERLAPPED      overlap                 = {0};
    DWORD           dwBytesRead             = 0;
    DWORD           dwEvent                 = 0;
    DWORD           dwError                 = 0;
    COMSTAT         cs                      = {0};
    HANDLE          hEvents[2]              = {0};
	BOOL			requestPending 			= FALSE;
	DWORD			dwPacketLength 			= 0;
	CHAR*			pcPacket			    = NULL;
    CHAR_LIST*      pHead                   = NULL;
    DWORD           dwQueueSize             = 0;
	DWORD           i                       = 0;

    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);
    
	
    if ((overlap.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL) {
        DISPLAY_ERROR("Error creating event in read thread");
    }
    hEvents[0] = overlap.hEvent;
    hEvents[1] = OpenEvent(DELETE | SYNCHRONIZE, FALSE, TEXT("disconnected"));


	
	
    while (pwd->bConnected) {

        SetCommMask(pwd->hPort, EV_RXCHAR);
        if (!WaitCommEvent(pwd->hPort, &dwEvent, &overlap)) {
            ProcessCommError(pwd->hPort);
        }
		if(!requestPending){
			RequestPacket(hWnd);
			requestPending = TRUE;
		}
        dwEvent = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
        if (dwEvent == WAIT_OBJECT_0 + 1) {
            // the connection was severed
            break;
        }
		
        ClearCommError(pwd->hPort, &dwError, &cs);
        
		
        // ensures that there is a character at the port
        if (cs.cbInQue) {  
            if (!ReadFile(pwd->hPort, psReadBuf, cs.cbInQue, 
                          &dwBytesRead, &overlap)) {
                // read is incomplete or had an error
                ProcessCommError(pwd->hPort);
                GetOverlappedResult(pwd->hThread, &overlap, &dwBytesRead, TRUE);
            }

            dwQueueSize = AddToBack(&pHead, psReadBuf, dwBytesRead);
            if (dwQueueSize >= 2) {
                dwPacketLength = GetFromList(pHead, 2);
            
                if (dwQueueSize >= dwPacketLength) {

                    pcPacket = RemoveFromFront(&pHead, dwPacketLength);
				    ProcessPacket(hWnd, pcPacket, dwPacketLength);
                    //MessageBox(NULL, TEXT("AAN"), TEXT(""), MB_OK);
                    memset(psReadBuf, 0, READ_BUFSIZE);
				    requestPending = FALSE;
                    free(pcPacket);
			    }
                InvalidateRect(hWnd, NULL, FALSE);
            }
        }
        ResetEvent(overlap.hEvent);
    }


    if (!PurgeComm(pwd->hPort, PURGE_RXCLEAR)) {
        DISPLAY_ERROR("Error purging read buffer");
    }
    CloseHandle(overlap.hEvent);
    return 0;
}

/*------------------------------------------------------------------------------
-- FUNCTION:    ProcessCommError
--
-- DATE:        Oct 13, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   VOID ProcessCommError(HANDLE hPort)
--                          hPort - the handle to the open port
--
-- RETURNS:     VOID.
--
-- NOTES:
--              Displays messages for various communication errors. Most of the
--              time, this function will process ERROR_IO_PENDING and return 
--              early since this is expected in overlapped I/O.
------------------------------------------------------------------------------*/
VOID ProcessCommError(HANDLE hPort) {
    DWORD dwError;

    if (GetLastError() == ERROR_IO_PENDING) {
        return;
    }
    ClearCommError(hPort, &dwError, NULL);

    switch (dwError) {

        case CE_BREAK:
            DISPLAY_ERROR("The hardware detected a break condition");
        case CE_FRAME:
            DISPLAY_ERROR("The hardware detected a framing error");
        case CE_OVERRUN:
            DISPLAY_ERROR("A character-buffer overrun has occurred. The next character is lost.");
        case CE_RXOVER:
            DISPLAY_ERROR("An input buffer overflow has occurred. There is either no room in the input buffer, or a character was received after the end-of-file (EOF) character");
        case CE_RXPARITY:
            DISPLAY_ERROR("The hardware detected a parity error");
        default:
            DISPLAY_ERROR("A communication error occured");
    }
}
