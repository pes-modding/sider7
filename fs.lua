-- fs : file-system utilities

local ffi = require('ffi')

local m = {}

if ffi ~= nil then
    ffi.cdef([[
typedef uint16_t WCHAR;
typedef uint16_t WORD;
typedef uint32_t UINT;
typedef uint32_t DWORD;
typedef uint64_t HANDLE;

typedef struct _FILETIME {
  DWORD dwLowDateTime;
  DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;

typedef struct _WIN32_FIND_DATAW {
  DWORD    dwFileAttributes;
  FILETIME ftCreationTime;
  FILETIME ftLastAccessTime;
  FILETIME ftLastWriteTime;
  DWORD    nFileSizeHigh;
  DWORD    nFileSizeLow;
  DWORD    dwReserved0;
  DWORD    dwReserved1;
  WCHAR    cFileName[260];
  WCHAR    cAlternateFileName[14];
  DWORD    dwFileType; // Obsolete. Do not use.
  DWORD    dwCreatorType; // Obsolete. Do not use
  WORD     wFinderFlags; // Obsolete. Do not use
} WIN32_FIND_DATAW, *PWIN32_FIND_DATAW, *LPWIN32_FIND_DATAW;

HANDLE FindFirstFileW(WCHAR *filename, LPWIN32_FIND_DATAW lpFindFileData);
BOOL FindNextFileW(HANDLE hFindFile, LPWIN32_FIND_DATAW lpFindFileData);
BOOL FindClose(HANDLE hFindFile);

typedef uint16_t* LPWSTR;
typedef char* LPSTR;
typedef char* LPCCH;
typedef uint8_t* LPBOOL;

int MultiByteToWideChar(UINT CodePage, DWORD dwFlags, char *lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar);
int WideCharToMultiByte(UINT CodePage, DWORD dwFlags, LPWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte,
        LPCCH lpDefaultChar, LPBOOL lpUsedDefaultChar);
    ]])
end

function m.find_files(pattern)
    -- sanity check: need FFI
    if ffi == nil then
        log('WARNING: find_files requires Luajit FFI. Set luajit.ext.enabled = 1 in your sider.ini')
        return function()
            return nil, nil
        end
    end

    local CP_UTF8 = 65001
    local FILE_ATTRIBUTE_DIRECTORY = 0x10

    local char_str = ffi.cast('char*', pattern)
    local wide_char_str = ffi.new('uint16_t[?]', 512)
    if ffi.C.MultiByteToWideChar(CP_UTF8, 0, char_str, #pattern, wide_char_str, 512) == 0 then
        error('MultiByteToWideChar returned error for pattern')
    end

    local find_data = ffi.new('WIN32_FIND_DATAW[?]', 1)
    local handle = ffi.C.FindFirstFileW(wide_char_str, find_data)
    if handle == -1 then
        return function()
            return nil, nil
        end
    end

    local res = true
    local null = ffi.new('void*')
    return function()
        if not res then
            ffi.C.FindClose(handle)
            return nil, nil
        end
        local filetype = 'file'
        if bit.band(find_data[0].dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) ~= 0 then
            filetype = 'dir'
        end
        local filename
        local filename_wide_str = find_data[0].cFileName
        local filename_mb_str = ffi.new('char[?]', 512)
        if ffi.C.WideCharToMultiByte(CP_UTF8, 0, filename_wide_str, -1, filename_mb_str, 512, null, null) > 0 then
            filename = ffi.string(filename_mb_str)
        else
            log(string.format('WARN: cannot convert with WideCharToMultiByte - skipping'))
        end
        res = ffi.C.FindNextFileW(handle, find_data)
        return filename, filetype
    end
end

return m
