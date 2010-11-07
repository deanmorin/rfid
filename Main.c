/*------------------------------------------------------------------------------
-- SOURCE FILE:     Main.c - Contains the WinMain() and WinProc()
--                                       functions for the Intelligent Terminal
--                                       Emulator.
--                      
-- PROGRAM:     RFID Reader - Enterprise Edition
--
-- FUNCTIONS:
--              int     WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, int)
--              LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM) 
--
--
-- DATE:        Oct 18, 2010
--
-- REVISIONS:   Nov 06, 2010
--              Removed any code not needed for the RFID reader.
--              (this file was copied from the Advanced Terminal Emulator Pro).
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- NOTES:
-- The main entry point for the program.
------------------------------------------------------------------------------*/

#include "Main.h"

/*------------------------------------------------------------------------------
-- FUNCTION:    WinMain
--
-- DATE:        Oct 18, 2010
--
-- REVISIONS:   Nov 06, 2010
--              Changed the window name.
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                                   PSTR szCmdLine, int iCmdShow)
--                          hInstance       - a handle to the current instance
--                          hPrevInstanc    - a handle to the previous instance
--                          szCmdLine       - the command line arguments
--                          iCmdShow        - specifies how the window should 
--                                            be shown
--
-- RETURNS:     If the function succeeds, terminating when it receives a WM_QUIT 
--              message, it should return the exit value contained in that 
--              message's wParam parameter. If the function terminates before 
--              entering the message loop, it should return zero.
--
-- NOTES:
--              WinMain is the conventional name used for the application entry
--              point. The standard message loop has been modified to poll the
--              an open serial port if there are no messages on the queue.o
------------------------------------------------------------------------------*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   PSTR szCmdLine, int iCmdShow) {
                
    static TCHAR    szAppName[] = TEXT("Main");
    HWND            hWnd        = NULL;
    MSG             msg         = {0};
    WNDCLASS        wndclass    = {0};
    PWNDDATA        pwd         = NULL;

    wndclass.style          = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc    = WndProc;
    wndclass.cbClsExtra     = 0;
    wndclass.cbWndExtra     = sizeof(PWNDDATA);
    wndclass.hInstance      = hInstance;
    wndclass.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground  = (HBRUSH) GetStockObject(BLACK_BRUSH);
    wndclass.lpszMenuName   = TEXT("MYMENU");
    wndclass.lpszClassName  = szAppName;

    if (!RegisterClass(&wndclass)) {
        MessageBox(NULL, TEXT("Upgrade your OS! Seriously!"),
                   szAppName, MB_ICONERROR);
        return 0;
    }

    hWnd = CreateWindow(szAppName,
                        TEXT("RFID Reader - Enterprise Edition (Trial Expired)"), 
                        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU 
                        | WS_MINIMIZEBOX,
                        CW_USEDEFAULT, CW_USEDEFAULT,
                        CW_USEDEFAULT, CW_USEDEFAULT,
                        NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, iCmdShow);
    UpdateWindow(hWnd);


    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage (&msg);
        DispatchMessage (&msg);
    }
    return msg.wParam;
}

/*------------------------------------------------------------------------------
-- FUNCTION:    WndProc
--
-- DATE:        Oct 18, 2010
--
-- REVISIONS:   Nov 06, 2010
--              Removed message handling that doesn't apply to this program.
--
-- DESIGNER:    Dean Morin
--
-- PROGRAMMER:  Dean Morin
--
-- INTERFACE:   LRESULT CALLBACK WndProc(HWND hWnd, UNIT message,
--                                       WPARAM wParam, LPARAM)
--                          hWnd    - the handle to the window
--                          message - the message
--                          wParam  - contents vary based on the message
                            lParam  - contents vary based on the message
--
-- RETURNS:     The return value is the result of the message processing and 
--              depends on the message sent.
--
-- NOTES:
--              The standard WndProc function.
------------------------------------------------------------------------------*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
                         WPARAM wParam, LPARAM lParam) {
                             
    PWNDDATA pwd = {0};
    pwd = (PWNDDATA) GetWindowLongPtr(hWnd, 0);

    switch (message) {

        case WM_CREATE:
            InitTerminal(hWnd);   
            return 0;
                     
        case WM_PAINT:
            Paint(hWnd);
            return 0;

        case WM_COMMAND:
            PerformMenuAction(hWnd, wParam);
            return 0;

        case WM_DESTROY:
            Disconnect(hWnd);
            PostQuitMessage(0);
            return 0;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
    }
}
