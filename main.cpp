#define UNICODE

#include <stdio.h>
#include <windows.h>
#include "sider.h"


HMODULE dll;
HOOKPROC addr;
HWND hWnd;
DWORD hookThreadId;

bool _inited(false);

int init();

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_DESTROY:
        case SIDER_MSG_EXIT:
            // Exit the application when the window closes
            append_to_log_(L"WindowProc:: uMsg=0x%x\n", uMsg);
            unsetHook();
            append_to_log_(L"WindowProc:: sider exiting\n");
            PostQuitMessage(0);
            return true;
    }
    return DefWindowProc(hwnd,uMsg,wParam,lParam);
}

bool InitApp(HINSTANCE hInstance, LPSTR lpCmdLine)
{
    WNDCLASSEX wcx;

    // cbSize - the size of the structure.
    wcx.cbSize = sizeof(WNDCLASSEX);
    wcx.style = CS_HREDRAW | CS_VREDRAW;
    wcx.lpfnWndProc = (WNDPROC)WindowProc;
    wcx.cbClsExtra = 0;
    wcx.cbWndExtra = 0;
    wcx.hInstance = hInstance;
    wcx.hIcon = LoadIcon(hInstance, L"si");
    wcx.hCursor = LoadCursor(NULL,IDC_ARROW);
    wcx.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wcx.lpszMenuName = NULL;
    wcx.lpszClassName = SIDERCLS;
    wcx.hIconSm = LoadIcon(hInstance, L"si");

    // Detect already running sider
    HWND hwndPrev;
    if ((hwndPrev = FindWindow(wcx.lpszClassName, NULL)) != NULL) {
        SetForegroundWindow(hwndPrev);
        return false;
    }

    // Register the class with Windows
    if(!RegisterClassEx(&wcx))
        return false;

    return true;
}

HWND BuildWindow(int nCmdShow)
{
    DWORD style, xstyle;
    HWND retval;

    style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    xstyle = WS_EX_LEFT;

    retval = CreateWindowEx(xstyle,
        L"SDR7CL64",      // class name
        L"Sider 7", // title for our window (appears in the titlebar)
        style,
        CW_USEDEFAULT,  // initial x coordinate
        CW_USEDEFAULT,  // initial y coordinate
        230, 70,   // width and height of the window
        NULL,           // no parent window.
        NULL,           // no menu
        NULL,           // no creator
        NULL);          // no extra data

    if (retval == NULL) return NULL;  // BAD.

    xstyle = WS_EX_LEFT;
    style = WS_CHILD | WS_VISIBLE;
    HWND heightLabel = CreateWindowEx(
            xstyle, L"Static", 
            L"Sider for Pro Evolution Soccer 2021", style,
            20, 10, 210, 50,
            retval, NULL, NULL, NULL);

    HGDIOBJ hObj = GetStockObject(DEFAULT_GUI_FONT);
    SendMessage(heightLabel, WM_SETFONT, (WPARAM)hObj, true);

    // Show the window
    if (start_minimized()) {
        ShowWindow(retval,
            SW_SHOWMINIMIZED|SW_SHOWMINNOACTIVE);
    }
    else {
        ShowWindow(retval,nCmdShow);
    }
    return retval; // return its handle for future use.
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    MSG msg; int retval;

    // init common controls
    //InitComCtls();

    if(InitApp(hInstance, lpCmdLine) == false)
        return 0;

    hWnd = BuildWindow(nCmdShow);
    if(hWnd == NULL) {
        return 0;
    }

    if (!_inited) {
        _inited = true;
        init();
    }

    // launch game, if specified in config
    wstring start_game;
    wstring work_dir;
    if (get_start_game(start_game)) {
        open_log_(L"start.game: %s\n", start_game.c_str());
        int pos = start_game.rfind(L"\\");
        if (pos != string::npos) {
            work_dir = start_game.substr(0, pos);
            log_(L"working directory: %s\n", work_dir.c_str());
        }
        close_log_();
        ShellExecute(NULL,L"open",start_game.c_str(),0,work_dir.c_str(),SW_SHOWNORMAL);
    }

    //SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
    while((retval = GetMessage(&msg,NULL,0,0)) != 0)
    {
        if(retval == -1)
            return 0;   // an error occured while getting a message

        // need to call this to make WS_TABSTOP work
        if (!IsDialogMessage(hWnd, &msg)) 
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return 0;
}
 
int init()
{
    wstring version;
    version.reserve(64);
    get_module_version(NULL,version);
    start_log_(L"Sider: version %s\n", version.c_str());
    setHook();
    log_(L"Main: Init DONE\n");
    close_log_();
	return 0;
}
