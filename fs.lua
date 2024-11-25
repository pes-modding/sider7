-- fs : file-system utilities

local ffi = require('ffi')
local C = ffi.C

local m = {}

ffi.cdef([[
typedef bool BOOL;
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
typedef uint16_t LPCWSTR;
typedef char* LPSTR;
typedef char* LPCCH;
typedef uint8_t* LPBOOL;

int MultiByteToWideChar(UINT CodePage, DWORD dwFlags, char *lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar);
int WideCharToMultiByte(UINT CodePage, DWORD dwFlags, LPWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte,
        LPCCH lpDefaultChar, LPBOOL lpUsedDefaultChar);
BOOL CreateDirectoryW(LPCWSTR *lpPathName, void *lpSecurityAttributes);
int GetLastError();
]])

local CP_UTF8 = 65001
local ERROR_UNKNOWN = -1
local ERROR_PATH_NOT_FOUND = 3
local ERROR_ALREADY_EXISTS = 183
local NULL = ffi.new("void*", nil)

local dir_errors = {
    [ERROR_UNKNOWN] = 'Unexpected error',
    [ERROR_ALREADY_EXISTS] = 'Directory already exists',
    [ERROR_PATH_NOT_FOUND] = 'Path not found',
}

local function make_dir(pathname)
    local char_str = ffi.cast('char*', pathname)
    local wide_char_str = ffi.new('uint16_t[?]', #pathname+1)

    if C.MultiByteToWideChar(CP_UTF8, 0, char_str, #pathname, wide_char_str, #pathname+1) == 0 then
        return 'MultiByteToWideChar returned error for pathname'
    end

    local ok = C.CreateDirectoryW(wide_char_str, NULL)
    if not ok then
        return C.GetLastError()
    end
end

local function make_dirs(pathname)
    -- start with outermost dir: try to create that first
    local err = make_dir(pathname)
    if err == nil then
        return
    end

    -- some other error: just return that
    if err ~= ERROR_PATH_NOT_FOUND then
        return err
    end

    -- try to create intermediate directories
    local parent_dir, _ = string.match(pathname, '(.*)[/\\](.+)')
    if not parent_dir then
        return ERROR_UNKNOWN
    end
    err = make_dirs(parent_dir)
    if err ~= nil and err ~= ERROR_ALREADY_EXISTS then
        return err
    end

    -- now try one more time
    return make_dir(pathname)
end

function m.make_dirs(pathname)
    local err = make_dirs(pathname)
    if err ~= nil then
        return dir_errors[err] or string.format('Error code: %s', err)
    end
end

function m.find_files(pattern)
    local CP_UTF8 = 65001
    local FILE_ATTRIBUTE_DIRECTORY = 0x10

    local char_str = ffi.cast('char*', pattern)
    local wide_char_str = ffi.new('uint16_t[?]', 512)
    if ffi.C.MultiByteToWideChar(CP_UTF8, 0, char_str, #pattern, wide_char_str, 512) == 0 then
        error('MultiByteToWideChar returned error for pattern')
    end

    local find_data = ffi.new('WIN32_FIND_DATAW[?]', 1)
    local handle = C.FindFirstFileW(wide_char_str, find_data)
    if handle == -1 then
        return function()
            return nil, nil
        end
    end

    local res = true
    local null = ffi.new('void*')
    return function()
        if not res then
            C.FindClose(handle)
            return nil, nil
        end
        local filetype = 'file'
        if bit.band(find_data[0].dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) ~= 0 then
            filetype = 'dir'
        end
        local filename
        local filename_wide_str = find_data[0].cFileName
        local filename_mb_str = ffi.new('char[?]', 512)
        if C.WideCharToMultiByte(CP_UTF8, 0, filename_wide_str, -1, filename_mb_str, 512, null, null) > 0 then
            filename = ffi.string(filename_mb_str)
        else
            log(string.format('WARN: cannot convert with WideCharToMultiByte - skipping'))
        end
        res = C.FindNextFileW(handle, find_data)
        return filename, filetype
    end
end

return m
