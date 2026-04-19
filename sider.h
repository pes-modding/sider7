#ifndef SIDER_H
#define SIDER_H

#include <string>
#include "d3d11.h"

using namespace std;

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
__declspec(dllexport) void truncate_applog_();
__declspec(dllexport) void applog_(const wchar_t *format, ...);
__declspec(dllexport) void get_module_version(HMODULE, wstring&);
__declspec(dllexport) bool get_start_game(wstring&);
__declspec(dllexport) bool start_minimized();

struct dx11_t {
    ID3D11Device *Device;
    ID3D11DeviceContext *Context;
    IDXGISwapChain *SwapChain;
    HWND Window;
    UINT Width;
    UINT Height;
};

struct SimpleVertex {
    float x;
    float y;
    float z;
    float w;
    float r;
    float g;
    float b;
    float a;
};

struct TexturedVertex {
    float x;
    float y;
    float z;
    float w;
    float tx;
    float ty;
    float tz;
    float tw;
};

struct TexConstants {
    float maxAlpha;
};

#endif
