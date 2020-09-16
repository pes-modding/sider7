#define UNICODE

#include <windows.h>
#include <string>

using namespace std;

__declspec(dllexport) void get_module_version(HMODULE hmodule, wstring& version)
{
    wchar_t full_path[MAX_PATH];
    memset(full_path, 0, sizeof(full_path));
    GetModuleFileName(hmodule, full_path, MAX_PATH);

    DWORD info_size = GetFileVersionInfoSize(full_path,NULL);
    if (info_size) {
         BYTE *data = new BYTE[info_size];
         if (GetFileVersionInfo(full_path, NULL, info_size, data)) {
             BYTE *buffer = NULL;
             UINT size = 0;
             if (VerQueryValue(data, L"\\", (LPVOID*)&buffer, &size)) {
                 if (size) {
                    VS_FIXEDFILEINFO *v = (VS_FIXEDFILEINFO *)buffer;
                    wchar_t ver_str[128];
                    memset(ver_str, 0, sizeof(ver_str));
                    swprintf(ver_str, L"%d.%d.%d (%d)",
                        (v->dwFileVersionMS >> 16) & 0xffff,
                        (v->dwFileVersionMS) & 0xffff,
                        (v->dwFileVersionLS >> 16) & 0xffff,
                        (v->dwFileVersionLS) & 0xffff
                    );
                    version = ver_str;
                 }
             }
         }
         delete [] data;
    }
}

