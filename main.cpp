#define UNICODE

#include <stdio.h>
#include <windows.h>
#include "sider.h"
#include <tlhelp32.h>
#include <string.h>
#include <stdlib.h>


typedef struct {
    DWORD processId;
    wchar_t processName[MAX_PATH];
} ProcessInfo;

typedef struct {
    DWORD pCoreCount;      
    DWORD eCoreCount;      
    DWORD_PTR pCoreMask;   
    DWORD_PTR eCoreMask;   
    DWORD totalCores;      
    BOOL isHybrid;         
} CPUCoreInfo;

int FindAllProcessesByName(const wchar_t* processName, ProcessInfo* processes, int maxCount);
BOOL SetProcessAffinityToPCores(DWORD processId, const CPUCoreInfo* coreInfo);
BOOL IsIntelCPUWithECores();
BOOL GetRealCoreInfo(CPUCoreInfo* coreInfo);
int SetProcessesToPCores(const wchar_t* processName);


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
            applog_(L"WindowProc:: uMsg=0x%x\n", uMsg);
            unsetHook();
            applog_(L"WindowProc:: sider exiting\n");
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
    wstring process_name;
    if (get_start_game(start_game)) {
        applog_(L"start.game: %s\n", start_game.c_str());
        wchar_t absolute_path[MAX_PATH];
        if (GetFullPathName(start_game.c_str(), MAX_PATH, absolute_path, NULL)) {
            wstring abs_game_path = absolute_path;
            
            int pos = abs_game_path.rfind(L"\\");
            if (pos != wstring::npos) {
                work_dir = abs_game_path.substr(0, pos);
                process_name = abs_game_path.substr(pos+1);
                applog_(L"working directory: %s\n", work_dir.c_str());
            }
            ShellExecute(NULL, L"open", abs_game_path.c_str(), 0, work_dir.c_str(), SW_SHOWNORMAL);
        }
    }


    int result = SetProcessesToPCores(process_name.c_str());
    applog_(L"Set processes to PCores: %d\n", result);  

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
    truncate_applog_();
    applog_(L"============================\n");
    applog_(L"Sider App: version %s\n", version.c_str());
    setHook();
    applog_(L"Main: Init DONE\n");
	return 0;
}

int SetProcessesToPCores(const wchar_t* processName) {
    if (!IsIntelCPUWithECores()) {
        return -1; 
    }
    
    CPUCoreInfo coreInfo;
    if (!GetRealCoreInfo(&coreInfo)) {
        return -4; 
    }
    
    if (!coreInfo.isHybrid) {
        return -1;
    }

    
    ProcessInfo processes[100];
    int foundCount = FindAllProcessesByName(processName, processes, 100);
    
    if (foundCount == 0) {
        return -2; 
    }
    
    int failCount = 0;
    for (int i = 0; i < foundCount; i++) {
        if (!SetProcessAffinityToPCores(processes[i].processId, &coreInfo)) {
            failCount++;
        }
    }
    
    return failCount;
}


int FindAllProcessesByName(const wchar_t* processName, ProcessInfo* processes, int maxCount) {
    HANDLE hSnapshot;
    PROCESSENTRY32W pe32;
    int foundCount = 0;
    
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }
    
    pe32.dwSize = sizeof(PROCESSENTRY32W);
    
    if (!Process32FirstW(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        return 0;
    }
    
    do {
        if (_wcsicmp(pe32.szExeFile, processName) == 0) {
            if (foundCount < maxCount) {
                processes[foundCount].processId = pe32.th32ProcessID;
                wcscpy_s(processes[foundCount].processName, MAX_PATH, pe32.szExeFile);
                foundCount++;
            } else {
                break; 
            }
        }
    } while (Process32NextW(hSnapshot, &pe32));
    
    CloseHandle(hSnapshot);
    return foundCount;
}

BOOL SetProcessAffinityToPCores(DWORD processId, const CPUCoreInfo* coreInfo) {
    HANDLE hProcess;
    DWORD_PTR processAffinityMask, systemAffinityMask;
    
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_SET_INFORMATION, 
                          FALSE, processId);
    if (hProcess == NULL) {
        return FALSE;
    }
    
    if (!GetProcessAffinityMask(hProcess, &processAffinityMask, &systemAffinityMask)) {
        CloseHandle(hProcess);
        return FALSE;
    }
    
    DWORD_PTR newAffinityMask = coreInfo->pCoreMask & systemAffinityMask;
    
    if (newAffinityMask == 0) {
        newAffinityMask = systemAffinityMask;
    }
    
    BOOL result = SetProcessAffinityMask(hProcess, newAffinityMask);
    CloseHandle(hProcess);
    return result;
}

BOOL GetRealCoreInfo(CPUCoreInfo* coreInfo) {
    DWORD bufferSize = 0;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX buffer = NULL;
    BOOL result = FALSE;
    
    memset(coreInfo, 0, sizeof(CPUCoreInfo));
    
    if (!GetLogicalProcessorInformationEx(RelationProcessorCore, NULL, &bufferSize)) {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            return FALSE;
        }
    }
    
    buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)malloc(bufferSize);
    if (buffer == NULL) {
        return FALSE;
    }
    
    if (!GetLogicalProcessorInformationEx(RelationProcessorCore, buffer, &bufferSize)) {
        free(buffer);
        return FALSE;
    }
    
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX current = buffer;
    DWORD offset = 0;
    
    while (offset < bufferSize) {
        if (current->Relationship == RelationProcessorCore) {
            DWORD_PTR coreMask = current->Processor.GroupMask[0].Mask;
            BYTE coreFlags = current->Processor.Flags;
            
            DWORD logicalProcessors = 0;
            DWORD_PTR tempMask = coreMask;
            while (tempMask) {
                if (tempMask & 1) logicalProcessors++;
                tempMask >>= 1;
            }
            
            if (logicalProcessors >= 2) {
                coreInfo->pCoreCount++;
                coreInfo->pCoreMask |= coreMask;
            } else {
                coreInfo->eCoreCount++;
                coreInfo->eCoreMask |= coreMask;
            }
            
            coreInfo->totalCores++;
        }
        
        offset += current->Size;
        current = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)((BYTE*)current + current->Size);
    }
    
    coreInfo->isHybrid = (coreInfo->pCoreCount > 0 && coreInfo->eCoreCount > 0);
    
    if (coreInfo->totalCores == 0) {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        DWORD totalLogicalProcessors = sysInfo.dwNumberOfProcessors;
        
        if (totalLogicalProcessors >= 8) {
            DWORD halfCores = totalLogicalProcessors / 2;
            
            for (DWORD i = 0; i < halfCores; i++) {
                coreInfo->pCoreMask |= ((DWORD_PTR)1 << i);
            }
            for (DWORD i = halfCores; i < totalLogicalProcessors; i++) {
                coreInfo->eCoreMask |= ((DWORD_PTR)1 << i);
            }
            
            coreInfo->pCoreCount = halfCores;
            coreInfo->eCoreCount = totalLogicalProcessors - halfCores;
            coreInfo->totalCores = totalLogicalProcessors;
            coreInfo->isHybrid = TRUE;
        }
    }
    
    result = (coreInfo->totalCores > 0);
    free(buffer);
    return result;
}

BOOL IsIntelCPUWithECores() {
    int cpuInfo[4];
    char vendor[13];
    
    __cpuid(cpuInfo, 0);
    memcpy(vendor, &cpuInfo[1], 4);
    memcpy(vendor + 4, &cpuInfo[3], 4);
    memcpy(vendor + 8, &cpuInfo[2], 4);
    vendor[12] = '\0';
    
    if (strcmp(vendor, "GenuineIntel") != 0) {
        return FALSE;
    }
    
    __cpuid(cpuInfo, 1);
    int family = ((cpuInfo[0] >> 8) & 0xF) + ((cpuInfo[0] >> 20) & 0xFF);
    int model = ((cpuInfo[0] >> 4) & 0xF) | ((cpuInfo[0] & 0xF0000) >> 12);
    
    if (family == 6 && model >= 0x97) {
        return TRUE;
    }
    
    return FALSE;
}