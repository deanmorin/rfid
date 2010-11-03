#include <stdio.h>
#include <windows.h>
/*
SOURCE FILE:     EscapeSequence.c - This contains all the functions necessary
--                                     to parse an escaped control sequence.
--
-- PROGRAM:     Advanced Terminal Emulator Pro
--
-- FUNCTIONS:
--              BOOL	CheckDigits(HWND hWnd, CHAR* psBuffer, 
--                                  DWORD length, DWORD *i);
--              BOOL	CheckDigitsQ(HWND hWnd, CHAR* psBuffer, 
--                                   DWORD length, DWORD *i);
--              BOOL	CheckDigitsSemi(HWND hWnd, CHAR* psBuffer, 
--                                      DWORD length, DWORD *i);
--              DWORD	GetDigit(CHAR* psBuffer, DWORD length, DWORD *i);
--              VOID    ProcessEsc(HWND hWnd, CHAR* psBuffer, 
--                                 DWORD dwBytesRead);
--              VOID    ProcessFont(HWND hWnd);
--              BOOL	ProcessParen(CHAR* psBuffer, DWORD length, DWORD *i);
--              BOOL	ProcessSquare(HWND hWnd, CHAR* psBuffer, 
--                                    DWORD length, DWORD *i);
--
--
-- DATE:        Oct 19, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Marcel Vangrootheest
--
-- PROGRAMMER:  Marcel Vangrootheest
--
-- NOTES:


FUNCTION:    ProcessEsc
--
-- DATE:        Oct 17, 2010
--
-- REVISIONS:   (Date and Description)
--
-- DESIGNER:    Marcel Vangrootheest
--
-- PROGRAMMER:  Marcel Vangrootheest
--
-- INTERFACE:   VOID ProcessEsc(HWND hWnd, CHAR* psBuffer, DWORD length)
--								hWnd		- Handle to the window
--								psBuffer	- Array of chars to be processed
--								length		- Number of chars to process
--
-- RETURNS:     VOID
--
-- NOTES:
--              Processes an Escape sequence. If a valid sequence is encountered
--				it will either end or send the rest of the array to ProcessRead.
--				If it is invalid it will send the invalid character plus the
--				rest of the array to ProcessRead. If it is incomplete it will
--				save the current sequence and another Read will be done to later
--				process the rest of the sequence.


/*---------------------------------------------------------------
--FUNCTION: 	DetectLRCError
--
--DATE:			Nov 2, 2010
--
--REVISIONS:	(Date and Description)
--
--DESIGNER:		Dean Morin, Marcel Vangrootheest
--
--Programer:	Ian Lee
--
--INTERFACE:	BOOL DetectLRCError(CHAR* pcPacket, DWORD dwLength)
--									pcPacket	- Frame to be processed
--									dwLength	- number of Bytes in frame
--
--
--RETURNS:		BOOL	true if an error is detected
--
--NOTES:
--
--				Performs LRC Error checking on the frame.  




----------------------------------------------------------------*/
BOOL DetectLRCError(CHAR* pcPacket, DWORD dwLength){
	DWORD i;
	
	char sum = 0x0;
	char ff = (char)0xFF;
	for(i = 0;i<dwLength - 2; i++){
		sum = sum ^ pcPacket[i];
	}
	if(sum != pcPacket[i++])
		return TRUE;
	
	sum = sum^ff;
	if(sum!= pcPacket[i++]){
		return TRUE;
	}
	return FALSE;
}


/*
int main(void){
	char a[]={(char)0x1,(char)0x9,(char)0x0,
			(char)0x3,(char)0x1,(char)0x41,
			(char) 0x1,(char)0x4A,(char)0xB5};
	
	
	if(DetectLRCError(a,9)){
		printf("error");
	}
	else 
		printf("no error");
	return 0;
	
}
*/
