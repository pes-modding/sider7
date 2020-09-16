#define UNICODE

#include <stdio.h>
#include <string>

#include "common.h"
#include "utf8.h"
#include "kmp.h"

extern wchar_t dll_log[MAX_PATH];
static FILE *file = NULL;
static CRITICAL_SECTION _log_cs;

#define LOG_BUF_LEN 0x200

void string_strip_quotes(wstring& s)
{
    static const wchar_t* chars = L" \t\n\r\"'";
    int e = s.find_last_not_of(chars);
    s.erase(e + 1);
    int b = s.find_first_not_of(chars);
    s.erase(0,b);
}

__declspec(dllexport) void log_(const wchar_t *format, ...)
{
    size_t count = LOG_BUF_LEN;
    wchar_t buffer[LOG_BUF_LEN];
    EnterCriticalSection(&_log_cs);
    if (file) {
        va_list params;
        va_start(params, format);
        vswprintf_s(buffer, count, format, params);
        va_end(params);
        char *encoded = Utf8::unicodeToUtf8(buffer);
        fwrite(encoded, strlen(encoded), 1, file);
        Utf8:free(encoded);
        fflush(file);
    }
    LeaveCriticalSection(&_log_cs);
}

__declspec(dllexport) void logu_(const char *format, ...)
{
    EnterCriticalSection(&_log_cs);
    if (file) {
        va_list params;
        va_start(params, format);
        vfprintf(file, format, params);
        va_end(params);
        fflush(file);
    }
    LeaveCriticalSection(&_log_cs);
}

__declspec(dllexport) void start_log_(const wchar_t *format, ...)
{
    InitializeCriticalSection(&_log_cs);
    EnterCriticalSection(&_log_cs);
    file = _wfopen(dll_log, L"wt");
    if (file) {
        va_list params;
        va_start(params, format);
        vfwprintf(file, format, params);
        va_end(params);
        fflush(file);
    }
    LeaveCriticalSection(&_log_cs);
}

__declspec(dllexport) void open_log_(const wchar_t *format, ...)
{
    InitializeCriticalSection(&_log_cs);
    EnterCriticalSection(&_log_cs);
    file = _wfopen(dll_log, L"a+");
    if (file) {
        va_list params;
        va_start(params, format);
        vfwprintf(file, format, params);
        va_end(params);
        fflush(file);
    }
    LeaveCriticalSection(&_log_cs);
}

__declspec(dllexport) void close_log_()
{
    EnterCriticalSection(&_log_cs);
    if (file) {
        fflush(file);
        fclose(file);
        file = NULL;
    }
    LeaveCriticalSection(&_log_cs);
    DeleteCriticalSection(&_log_cs);
}

__declspec(dllexport) void append_to_log_(const wchar_t *format, ...)
{
    InitializeCriticalSection(&_log_cs);
    EnterCriticalSection(&_log_cs);
    file = _wfopen(dll_log, L"a+");
    if (file) {
        va_list params;
        va_start(params, format);
        vfwprintf(file, format, params);
        va_end(params);
        fflush(file);
        fclose(file);
        file = NULL;
    }
    LeaveCriticalSection(&_log_cs);
}

BYTE* get_target_addr(BYTE* call_location)
{
    if (call_location) {
        BYTE* bptr = call_location;
        DWORD protection = 0;
        DWORD newProtection = PAGE_EXECUTE_READWRITE;
        if (VirtualProtect(bptr, 8, newProtection, &protection)) {
            // get original target
            DWORD* ptr = (DWORD*)(call_location + 1);
            return call_location + ptr[0] + 5;
        }
    }
    return NULL;
}

void hook_call_point(
    DWORD addr, void* func, int codeShift, int numNops, bool addRetn)
{
    DWORD target = (DWORD)func + codeShift;
	if (addr && target)
	{
	    BYTE* bptr = (BYTE*)addr;
	    DWORD protection = 0;
	    DWORD newProtection = PAGE_EXECUTE_READWRITE;
	    if (VirtualProtect(bptr, 16, newProtection, &protection)) {
	        bptr[0] = 0xe8;
	        DWORD* ptr = (DWORD*)(addr + 1);
	        ptr[0] = target - (DWORD)(addr + 5);
            // padding with NOPs
            for (int i=0; i<numNops; i++) bptr[5+i] = 0x90;
            if (addRetn)
                bptr[5+numNops]=0xc3;
	        log_(L"Function (%08x) HOOKED at address (%08x)\n", target, addr);
	    }
	}
}

BYTE* check_hint(BYTE *base, LONGLONG max_offset, BYTE *frag, size_t frag_len, void *hint)
{
    BYTE *p = base;
    BYTE *max_p = base + max_offset;
    BYTE *hp = (BYTE*)hint;
    if (p <= hp && hp + frag_len <= max_p) {
        if (memcmp(hp, frag, frag_len)==0) {
            return hp;
        }
    }
    return NULL;
}

BYTE* find_code_frag(BYTE *base, LONGLONG max_offset, BYTE *frag, size_t frag_len)
{
    BYTE *p = base;
    BYTE *max_p = base + max_offset;
    //logu_("searching range: %p : %p for %lu bytes\n", p, max_p, frag_len);
    return (BYTE*)kmp_search((char*)frag, frag_len, (char*)p, (char*)max_p);
    /*
    while (p < max_p && memcmp(p, frag, frag_len)!=0) {
        p += 1;
    }
    if (p < max_p) {
        return p;
    }
    return NULL;
    */
}

void patch_at_location(BYTE *addr, void *patch, size_t patch_len)
{
    if (addr) {
	    BYTE* bptr = addr;
	    DWORD protection = 0;
	    DWORD newProtection = PAGE_EXECUTE_READWRITE;
	    if (VirtualProtect(bptr, patch_len, newProtection, &protection)) {
            memcpy(addr, patch, patch_len);
            log_(L"Patch (size=%d) installed at (%p)\n", patch_len, addr);
        }
        else {
            log_(L"Problem with VirtualProtect at: %p\n", addr);
        }
	}
}

