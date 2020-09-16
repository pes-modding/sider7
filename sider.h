#ifndef SIDER_H
#define SIDER_H

#include <string>

using namespace std;

#define SIDER_FM L"Local\\sider-6"

#define SIDERCLS L"SDR7CL64"
#define SIDER_MSG_EXIT WM_USER + 1

__declspec(dllexport) void setHook();
__declspec(dllexport) void setHook1();
__declspec(dllexport) void unsetHook(bool all=true);
__declspec(dllexport) void log_(const wchar_t *format, ...);
__declspec(dllexport) void logu_(const char *format, ...);
__declspec(dllexport) void start_log_(const wchar_t *format, ...);
__declspec(dllexport) void open_log_(const wchar_t *format, ...);
__declspec(dllexport) void close_log_();
__declspec(dllexport) void append_to_log_(const wchar_t *format, ...);
__declspec(dllexport) void get_module_version(HMODULE, wstring&);
__declspec(dllexport) bool get_start_game(wstring&);
__declspec(dllexport) bool start_minimized();

#endif
