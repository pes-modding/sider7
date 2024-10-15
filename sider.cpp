#define UNICODE

//#include "stdafx.h"
#include <windows.h>
#include <shellapi.h>
#include <psapi.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include "zlib.h"
#include "imageutil.h"
#include "config.h"
#include "sider.h"
#include "utf8.h"
#include "common.h"
#include "patterns.h"
#include "memlib.h"
#include "fslib.h"
#include "kmp.h"
#include "libz.h"
#include "kitinfo.h"
#include "audio.h"
#include "regs.h"

#define XINPUT_USE_9_1_0
#define DIRECTINPUT_VERSION 0x0800
#define SAFE_RELEASE(x) if (x) { x->Release(); x = NULL; }

#include "d3d11.h"
//#include "d3dcompiler.h"
#include "FW1FontWrapper.h"
#include "dinput.h"
#include "xinput.h"
#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"

#include "lua.hpp"
#include "lauxlib.h"
#include "lualib.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
//#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
//#pragma comment(lib, "xinput.lib")
#pragma comment(lib, "xinput9_1_0.lib")

#ifndef LUA_OK
#define LUA_OK 0
#endif

#define DBG(n) if (_config->_debug & n)

#define smaller(a,b) ((a<b)?a:b)

// "sdr5" magic string
#define MAGIC 0x35726473

//#define KNOWN_FILENAME "common\\etc\\pesdb\\Coach.bin"
#define KNOWN_FILENAME "Fox\\Scripts\\Gr\\init.lua"
char _file_to_lookup[0x80];
size_t _file_to_lookup_size = 0;

using namespace std;

CRITICAL_SECTION _cs;
CRITICAL_SECTION _tcs;
lua_State *L = NULL;

int _memory_lib_index = 0;
int _fs_lib_index = 0;
int _audio_lib_index = 0;

struct FILE_HANDLE_INFO {
    HANDLE handle;
    DWORD size;
    DWORD sizeHigh;
    DWORD currentOffset;
    DWORD currentOffsetHigh;
    DWORD padding[4];
};

struct FILE_LOAD_INFO {
    BYTE *vtable;
    FILE_HANDLE_INFO *file_handle_info;
    DWORD dw0[2];
    LONGLONG type;
    DWORD dw1[4];
    char *cpk_filename;
    LONGLONG cpk_filesize;
    LONGLONG filesize;
    DWORD dw2[2];
    LONGLONG offset_in_cpk;
    DWORD total_bytes_to_read;
    DWORD max_bytes_to_read;
    DWORD bytes_to_read;
    DWORD bytes_read_so_far;
    DWORD dw3[2];
    LONGLONG buffer_size;
    BYTE *buffer;
    BYTE *buffer2;
};

struct READ_STRUCT {
    BYTE b0[0x170];
    LONGLONG filesize;
    FILE_HANDLE_INFO *fileinfo;
    union {
        struct {
            DWORD low;
            DWORD high;
        } parts;
        LONGLONG full;
    } offset;
    BYTE b1[0x20];
    char filename[0x80];
};

struct BUFFER_INFO {
    LONGLONG data0;
    BYTE *someptr;
    LONGLONG data1;
    BYTE *buffer;
    BYTE *buffer2;
    BYTE b0[0x1c0];
    char *filename;
};

struct FILE_INFO {
    DWORD size;
    DWORD size_uncompressed;
    LONGLONG offset_in_cpk;
};

struct STAD_STRUCT {
    DWORD stadium;
    DWORD timeofday;
    DWORD weather;
    DWORD season;
};

struct SHIRTCOLOR_STRUCT {
    BYTE index;
    BYTE unknown1;
    BYTE color1[3];
    BYTE color2[3];
};

struct TEAM_INFO_STRUCT {
    DWORD team_id_encoded;
    BYTE team_name[0x46];
    BYTE team_abbr[4];
    BYTE unknown1[2];
    BYTE unknown2[0x588];
    SHIRTCOLOR_STRUCT players[2];
    SHIRTCOLOR_STRUCT goalkeepers[1];
    SHIRTCOLOR_STRUCT extra_players[7];
    BYTE unknown3[0x68];
};

struct MATCH_INFO_STRUCT {
    DWORD dw0;
    DWORD dw1;
    WORD match_id;
    WORD tournament_id_encoded;
    BYTE match_leg;
    BYTE unknown0[3];
    BYTE match_info;
    BYTE unknown1[3];
    DWORD unknown2;
    DWORD difficulty;
    BYTE match_time;
    BYTE unknown3[3];
    DWORD unknown4[3];
    BYTE extra_time_choice;
    BYTE unknown7;
    BYTE unknown8;
    BYTE penalties;
    BYTE unknown_zero;
    BYTE num_subs; //subs
    BYTE num_subs_et; //subs in extra time
    BYTE db0x17;
    WORD stadium_choice;
    WORD unknown5;
    DWORD timeofday_choice; // 0-day, 1-night
    DWORD weather_choice;   // 0-fine, 1-rainy, 2-snow
    DWORD weather_effects; // 0-nothing, 1-later, 2-now
    DWORD season_choice; // 0-summer, 1-winter
    DWORD unknown9[10];
    struct STAD_STRUCT stad;
    BYTE unknown10[0xa0];
    BYTE home_player_kit_id;
    BYTE home_player_kit_id_unknown[3];
    BYTE away_player_kit_id;
    BYTE away_player_kit_id_unknown[3];
    BYTE home_gk_kit_id;
    BYTE home_gk_kit_id_unknown[3];
    BYTE away_gk_kit_id;
    BYTE away_gk_kit_id_unknown[3];
    DWORD unknown11[2];
    TEAM_INFO_STRUCT home;
    TEAM_INFO_STRUCT away;
};

struct STAD_INFO_STRUCT {
    DWORD unknown0;
    WORD id;
    WORD unknown1;
    char name[0xac];
};

struct TROPHY_TABLE_ENTRY {
    WORD tournament_id;
    WORD dw0;
    DWORD trophy_id;
};

struct SCOREBOARD_INFO2 {
    BYTE is_clock_running_normally;
    BYTE unknown1;
    BYTE some_counter;
    BYTE unknown2;
    BYTE unknown3[0x84];
    BYTE match_stage; // 0x05 - live game, 0x0c - kick-off, 0x10 - pk shootout?
    BYTE unknown4[3];
    DWORD unknown5[4];
    BYTE unknown6;
    BYTE is_paused;
    BYTE home_score;
    BYTE away_score;
    BYTE unknown7[0x80];
    DWORD unknown8[3];
    BYTE home_penalty_score;
    BYTE away_penalty_score;
    BYTE unknown9;
    BYTE unknown10;
    DWORD home_penalty_kicks[5]; // 0 - not taken yet, 1 - scored, 2 - missed
    DWORD away_penalty_kicks[5]; // 0 - not taken yet, 1 - scored, 2 - missed
};

struct SCOREBOARD_INFO {
    BYTE *vtable;
    BYTE unknown1[0x18];
    BYTE is_paused;
    BYTE unknown2;
    BYTE is_clock_running;
    BYTE unknown3;
    DWORD unknown4[5];
    SCOREBOARD_INFO2 *sci2;
    BYTE unknown5[0xe0];
    DWORD home_score;
    DWORD away_score;
    DWORD unknown6;
    DWORD clock_minutes;
    DWORD clock_seconds;
    DWORD unknown7;
    BYTE is_injury_time;
    BYTE unknown8[3];
    DWORD clock_minutes_again;
    DWORD clock_seconds_again;
    DWORD added_minutes;
};

struct SCHEDULE_ENTRY {
    DWORD unknown1;
    WORD tournament_id;
    BYTE match_info;
    BYTE unknown2;
    DWORD unknown3[3];
    DWORD home_team_encoded;
    DWORD away_team_encoded;
    DWORD unknown4;
};

#define TT_LEN 0x148
TROPHY_TABLE_ENTRY _trophy_table[TT_LEN];
TROPHY_TABLE_ENTRY _trophy_map[TT_LEN];
int64_t _trophy_table_copy_count;

BYTE _variations[128];

WORD _tournament_id = 0xffff;
char _ball_name[256];
char _stadium_name[256];
int _stadium_choice_count = 0;
struct STAD_INFO_STRUCT _stadium_info;
MATCH_INFO_STRUCT *_mi = NULL;
void *_uniparam_base = NULL;
KIT_STATUS_INFO *_ksi = NULL;
TEAM_INFO_STRUCT *_home_team_info = NULL;
TEAM_INFO_STRUCT *_away_team_info = NULL;
DWORD _edit_team_id;

extern "C" SCOREBOARD_INFO *_sci = NULL;
int _stats_table_index = 0;
int _match_lib_index = 0;
bool _has_on_frame(false);

// home team encoded-id offset: 0x104
// home team name offset:       0x108
// away team encoded-id offset: 0x624
// away team name offset:       0x628

DWORD decode_team_id(DWORD team_id_encoded);
void play_overlay_toggle_sound();
int get_context_field_int(const char *name);
void set_context_field_lightuserdata(const char *name, void *p);
void set_context_field_boolean(const char *name, bool value);
void set_context_field_int(const char *name, int value);
void set_context_field_nil(const char *name);
void clear_context();
void lua_reload_modified_modules();

const char *_context_fields[] = {
    "match_id", "match_info", "match_leg",// "match_time",
    "away_team", "home_team", "stadium_choice", "stadium",
    "weather", "weather_effects", "timeofday", "season",
    "tournament_id", "mis", "sci", "difficulty", "extra_time", "penalties",
    "substitutions", "substitutions_in_extra_time",
};
size_t _context_fields_count = sizeof(_context_fields)/sizeof(const char *);

struct uniparam_info_t {
    BYTE* block;
    DWORD size;
    char* name;
};
struct uniparam_team_t {
    bool has_1st;
    bool has_2nd;
    bool has_gk1st;
};
bool _dummified(false);

/**
#define LOOKUP_CACHE_KEY_LEN 512
struct lookup_cache_value_t {
    char key[LOOKUP_CACHE_KEY_LEN];
    wstring *value;
};
class lookup_cache_t {
public:
    lookup_cache_value_t *_data;
    size_t _data_count;
    size_t _next;
    CRITICAL_SECTION _cs;

    lookup_cache_t(size_t data_count) : _data_count(data_count), _next(0) {
        InitializeCriticalSection(&_cs);
        _data = (lookup_cache_value_t*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, _data_count*sizeof(lookup_cache_value_t));
    }
    ~lookup_cache_t() {
        DeleteCriticalSection(&_cs);
        HeapFree(GetProcessHeap(), 0, _data);
    }
    inline void store(size_t i, const char *key, wstring *value) {
        _data[i].key[0] = '\0';
        strncat(_data[i].key, key, LOOKUP_CACHE_KEY_LEN-1);
        _data[i].value = value;
    }
    bool get(const char *key, wstring **value) {
        lock_t lock(&_cs);
        for (int i=0; i<_data_count; i++) {
            int j = _data_count - 1 - (_data_count - _next + i) % _data_count;
            if (strncmp(_data[j].key, key, LOOKUP_CACHE_KEY_LEN)==0) {
                *value = _data[j].value;
                return true;
            }
        }
        return false;
    }
    void put(const char *key, wstring *value) {
        lock_t lock(&_cs);
        for (int i=0; i<_data_count; i++) {
            int j = _data_count - 1 - (_data_count - _next + i) % _data_count;
            if (strncmp(_data[j].key, key, LOOKUP_CACHE_KEY_LEN)==0) {
                store(j, key, value);
                return;
            }
        }
        store(_next, key, value);
        _next = (_next + 1) % _data_count;
    }
    size_t size() {
        return _data_count;
    }
};

//typedef unordered_map<string,wstring*> lookup_cache_t;
lookup_cache_t *_lookup_cache(NULL);
**/

//typedef LONGLONG (*pfn_alloc_mem_t)(BUFFER_INFO *bi, LONGLONG size);
//pfn_alloc_mem_t _org_alloc_mem;

BYTE* get_target_location(BYTE *call_location);
BYTE* get_target_location2(BYTE *rel_offs_location);
void HookXInputGetState();

LRESULT CALLBACK sider_keyboard_proc(int code, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK sider_foreground_idle_proc(int code, WPARAM wParam, LPARAM lParam);

typedef HRESULT (*PFN_CreateDXGIFactory1)(REFIID riid, void **ppFactory);
typedef HRESULT (*PFN_IDXGIFactory1_CreateSwapChain)(IDXGIFactory1 *pFactory, IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC *pDesc, IDXGISwapChain **ppSwapChain);
typedef HRESULT (*PFN_IDXGISwapChain_Present)(IDXGISwapChain *swapChain, UINT SyncInterval, UINT Flags);
PFN_CreateDXGIFactory1 _org_CreateDXGIFactory1;
PFN_IDXGIFactory1_CreateSwapChain _org_CreateSwapChain;
PFN_IDXGISwapChain_Present _org_Present(NULL);

HRESULT sider_CreateDXGIFactory1(REFIID riid, void **ppFactory);
HRESULT sider_CreateSwapChain(IDXGIFactory1 *pFactory, IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC *pDesc, IDXGISwapChain **ppSwapChain);
HRESULT sider_Present(IDXGISwapChain *swapChain, UINT SyncInterval, UINT Flags);

typedef DWORD (*PFN_XInputGetState)(DWORD dwUserIndex, XINPUT_STATE *pState);
typedef HRESULT (*PFN_IDirectInput8_CreateDevice)(IDirectInput8 *self, REFGUID rguid, LPDIRECTINPUTDEVICE * lplpDirectInputDevice, LPUNKNOWN pUnkOuter);
typedef HRESULT (*PFN_IDirectInputDevice8_GetDeviceState)(IDirectInputDevice8 *self, DWORD cbData, LPVOID lpvData);
//typedef HRESULT (*PFN_IDirectInputDevice8_GetDeviceData)(IDirectInputDevice8 *self, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod,
//         LPDWORD pdwInOut, DWORD dwFlags);
//typedef HRESULT (*PFN_IDirectInputDevice8_Poll)(IDirectInputDevice8 *self);
PFN_XInputGetState _org_XInputGetState;
PFN_IDirectInput8_CreateDevice _org_CreateDevice;
PFN_IDirectInputDevice8_GetDeviceState _org_GetDeviceStateKeyboard;
PFN_IDirectInputDevice8_GetDeviceState _org_GetDeviceStateGamepad;
//PFN_IDirectInputDevice8_GetDeviceData _org_GetDeviceData;
//PFN_IDirectInputDevice8_Poll _org_Poll;
DWORD sider_XInputGetState(DWORD dwUserIndex, XINPUT_STATE *pState);
HRESULT sider_CreateDevice(IDirectInput8 *self, REFGUID rguid, LPDIRECTINPUTDEVICE * lplpDirectInputDevice, LPUNKNOWN pUnkOuter);
HRESULT sider_GetDeviceStateKeyboard(IDirectInputDevice8 *self, DWORD cbData, LPVOID lpvData);
HRESULT sider_GetDeviceStateGamepad(IDirectInputDevice8 *self, DWORD cbData, LPVOID lpvData);
//HRESULT sider_GetDeviceData(IDirectInputDevice8 *self, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags);
//HRESULT sider_Poll(IDirectInputDevice8 *self);
map<BYTE**, BYTE*> _vtables;

BOOL sider_device_enum_callback(LPCDIDEVICEINSTANCE lppdi, LPVOID pvRef);
BOOL sider_device_enum_callback(LPCDIDEVICEINSTANCE lppdi, LPVOID pvRef);
BOOL sider_object_enum_callback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);
void init_direct_input();
void enumerate_controllers();
BYTE **_xinput_get_state_holder(NULL);

ID3D11Device *_device(NULL);
ID3D11DeviceContext *_device_context(NULL);
IDXGISwapChain *_swap_chain(NULL);
ID3D11Resource *g_texture(NULL);
ID3D11ShaderResourceView *g_textureView(NULL);
ID3D11SamplerState *g_pSamplerLinear(NULL);

struct overlay_image_t {
    bool to_clear;
    bool have;
    int source_width;
    int source_height;
    int width;
    int height;
    int vmargin;
    int hmargin;
    char *filepath;
};

struct layout_t {
    int image_width;
    int image_height;
    bool has_image_ar;
    bool has_image_hmargin;
    bool has_image_vmargin;
    float image_aspect_ratio;
    float image_hmargin;
    float image_vmargin;
};

overlay_image_t _overlay_image;

struct dx11_t {
    ID3D11Device *Device;
    ID3D11DeviceContext *Context;
    IDXGISwapChain *SwapChain;
    HWND Window;
    UINT Width;
    UINT Height;
};
dx11_t DX11;

IDirectInput8 *g_IDirectInput8(NULL);
IDirectInputDevice8 *g_IDirectInputDevice8(NULL);
GUID g_controller_guid_instance;
bool _has_controller(false);
bool _enumerated_controllers(false);
bool _controller_prepped(false);
bool _controller_poll_initialized(false);
bool _controller_poll(false);
int _controller_poll_delay(0);
HANDLE _controller_poll_handle(INVALID_HANDLE_VALUE);

DIDATAFORMAT _data_format;
BYTE _prev_controller_buttons[64];
BYTE _controller_buttons[64];
vector<DIDEVICEOBJECTINSTANCE> _di_objects;

struct di_input_t {
    DWORD dwOfs;
    DWORD dwType;
    int what;
};

di_input_t _di_overlay_toggle1;
di_input_t _di_overlay_toggle2;
di_input_t _di_module_switch;
di_input_t _di_module_switch_prev;

struct di_change_t {
    int state;
    DWORD dwType;
};

/* similar to XINPUT_STATE, but with values adjusted for sider use */
struct xi_state_t {
    BYTE buttons[10];
    BYTE left_trigger;
    BYTE right_trigger;
    int8_t left_stick_x;
    int8_t left_stick_y;
    int8_t right_stick_x;
    int8_t right_stick_y;
    BYTE dpad;
};
xi_state_t _prev_xi_state;

struct xi_change_t {
    int state;
    int what;
};
xi_change_t _xi_changes[32];
size_t _xi_changes_len = 0;

bool _toggle_sequence_on(false);

IFW1Factory *g_pFW1Factory;
IFW1FontWrapper *g_pFontWrapper;
float _font_size = 20.0f;

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

SimpleVertex g_vertices[] =
{
    { -1.0f, 1.0f, 0.5f, 1.f, 0.2f, 0.4f, 0.2f, 0.8f },
    { 1.0f, 1.0f, 0.5f, 1.f, 0.2f, 0.4f, 0.2f, 0.8f },
    { 1.0f, -1.0f, 0.5f, 1.f, 0.2f, 0.4f, 0.2f, 0.8f },
    { 1.0f, -1.0f, 0.5f, 1.f, 0.2f, 0.4f, 0.2f, 0.8f },
    { -1.0f, -1.0f, 0.5f, 1.f, 0.2f, 0.4f, 0.2f, 0.8f },
    { -1.0f, 1.0f, 0.5f, 1.f, 0.2f, 0.4f, 0.2f, 0.8f },
};

static const TexturedVertex g_texVertices[] =
{
    { -1.f, 1.f, 0.4f, 1.f, 0.f, 0.f, 0.f, 0.f },
    { 1.f, 1.f, 0.4f, 1.f, 1.f, 0.f, 0.f, 0.f },
    { 1.f, -1.f, 0.4f, 1.f, 1.f, 1.f, 0.f, 0.f },
    { 1.f, -1.f, 0.4f, 1.f, 1.f, 1.f, 0.f, 0.f },
    { -1.f, -1.f, 0.4f, 1.f, 0.f, 1.f, 0.f, 0.f },
    { -1.f, 1.f, 0.4f, 1.f, 0.f, 0.f, 0.f, 0.f },
};

struct TexConstants {
    float maxAlpha;
};

TexConstants g_constants;

ID3D11BlendState* g_pBlendState = NULL;
ID3D11InputLayout*          g_pInputLayout = NULL;
ID3D11InputLayout*          g_pTexInputLayout = NULL;
ID3D11Buffer*               g_pConstantBuffer = NULL;
ID3D11Buffer*               g_pVertexBuffer = NULL;
ID3D11Buffer*               g_pTexVertexBuffer = NULL;
ID3D11RenderTargetView*     g_pRenderTargetView = NULL;
ID3D11VertexShader*         g_pVertexShader = NULL;
ID3D11VertexShader*         g_pTexVertexShader = NULL;
ID3D11PixelShader*          g_pPixelShader = NULL;
ID3D11PixelShader*          g_pTexPixelShader = NULL;
HRESULT hr = S_OK;

char* g_strVS =
    "void VS( in float4 posIn : POSITION,\n"
    "         out float4 posOut : SV_Position )\n"
    "{\n"
    "    // Output the vertex position, unchanged\n"
    "    posOut = posIn;\n"
    "}\n";

char* g_strTexVS =
    "struct VS_INPUT\n"
    "{\n"
    "    float4 Pos : POSITION;\n"
    "    float4 Tex : TEXCOORD0;\n"
    "};\n"
    "struct PS_INPUT\n"
    "{\n"
    "    float4 Pos : SV_POSITION;\n"
    "    float4 Tex : TEXCOORD0;\n"
    "};\n"
    "PS_INPUT VStex( VS_INPUT input )\n"
    "{\n"
    "    PS_INPUT output = (PS_INPUT)0;\n"
    "    output.Pos = input.Pos;\n"
    "    output.Tex = input.Tex;\n"
    "    return output;\n"
    "}\n";

char* g_strPS =
    "void PS( out float4 colorOut : SV_Target )\n"
    "{\n"
    "    colorOut = float4( %0.2f, %0.2f, %0.2f, %0.2f );\n"
    "}\n";

char *g_strTexPS =
    "Texture2D tx2D : register( t0 );\n"
    "SamplerState samLinear : register( s0 );\n"
    "\n"
    "struct PS_INPUT\n"
    "{\n"
    "    float4 Pos : SV_POSITION;\n"
    "    float4 Tex : TEXCOORD0;\n"
    "};\n"
    "\n"
    "float4 PStex( PS_INPUT input) : SV_Target\n"
    "{\n"
    "    float4 clr = tx2D.Sample( samLinear, input.Tex.xy );\n"
    "    clr.a = min(%0.3f, clr.a);\n"
    "    return clr;\n"
    "    //return float4(input.Tex.x,input.Tex.y,0.0f,0.4f);\n"
    "    //return float4(1.0f,0.0f,0.0f,0.4f);\n"
    "}\n";

extern "C" BOOL sider_read_file(
    HANDLE       hFile,
    LPVOID       lpBuffer,
    DWORD        nNumberOfBytesToRead,
    LPDWORD      lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped,
    struct READ_STRUCT *rs);

extern "C" void sider_get_size(char *filename, struct FILE_INFO *fi);

extern "C" BOOL sider_read_file_hk(
    HANDLE       hFile,
    LPVOID       lpBuffer,
    DWORD        nNumberOfBytesToRead,
    LPDWORD      lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped);

extern "C" void sider_get_size_hk();

extern "C" void sider_extend_cpk_hk();

extern "C" void sider_mem_copy(BYTE *dst, LONGLONG dst_len, BYTE *src, LONGLONG src_len, BYTE **rsp);

extern "C" void sider_mem_copy_hk();

extern "C" void sider_lookup_file(LONGLONG p1, LONGLONG p2, char *filename);

extern "C" void sider_lookup_file_hk();

extern "C" void sider_set_team_id(DWORD *dest, TEAM_INFO_STRUCT *team_info, DWORD offset);

extern "C" void sider_set_team_id_hk();

extern "C" void sider_set_settings(STAD_STRUCT *dest_ss, STAD_STRUCT *src_ss);

extern "C" void sider_set_settings_hk();

extern "C" DWORD sider_trophy_check(DWORD tournament_id);

extern "C" DWORD sider_trophy_check_hk(DWORD tournament_id);

extern "C" DWORD sider_trophy_check2_hk(DWORD tournament_id);

extern "C" void sider_context_reset();

extern "C" void sider_context_reset_hk();

extern "C" void sider_free_select_hk();

extern "C" void sider_free_select(BYTE *controller_restriction);

extern "C" void sider_trophy_table(TROPHY_TABLE_ENTRY *tt);

extern "C" void sider_trophy_table_hk();

extern "C" char* sider_ball_name(char *ball_name);

extern "C" void sider_ball_name_hk();

extern "C" char* sider_stadium_name(STAD_INFO_STRUCT *stad_info, LONGLONG rdx, LONGLONG ptr, SCHEDULE_ENTRY *se);

extern "C" void sider_stadium_name_hk();

extern "C" STAD_INFO_STRUCT* sider_def_stadium_name(DWORD stadium_id);

extern "C" void sider_def_stadium_name_hk();

extern "C" void sider_set_stadium_choice(MATCH_INFO_STRUCT *mi, WORD stadium_id);

extern "C" void sider_set_stadium_choice_hk();

extern "C" void sider_check_kit_choice(MATCH_INFO_STRUCT *mi, DWORD home_or_away);

extern "C" void sider_check_kit_choice_hk();

extern "C" DWORD sider_data_ready(FILE_LOAD_INFO *fli);

extern "C" void sider_data_ready_hk();

extern "C" void sider_kit_status(KIT_STATUS_INFO *ksi, TASK_UNIFORM_IMPL *tu_impl);

extern "C" void sider_kit_status_hk();

extern "C" void sider_set_team_for_kits(KIT_STATUS_INFO *ksi, DWORD team_id_encoded, LONGLONG r8, DWORD *which);

extern "C" void sider_set_team_for_kits_hk();

extern "C" void sider_clear_team_for_kits(KIT_STATUS_INFO *ksi, DWORD *which);

extern "C" void sider_clear_team_for_kits_hk();

extern "C" BYTE* sider_loaded_uniparam(BYTE *uniparam);

extern "C" void sider_loaded_uniparam_hk();

extern "C" void sider_copy_clock_hk();

extern "C" void sider_clear_sc(SCOREBOARD_INFO *sci);

extern "C" void sider_clear_sc_hk();

extern "C" void sider_set_edit_team_id_hk();

extern "C" void sider_set_edit_team_id(DWORD team_id_encoded);

extern "C" void sider_custom_event_rbx_hk();

extern "C" void sider_custom_event(uint16_t param, REGISTERS *regs);

static DWORD dwThreadId;
static DWORD hookingThreadId = 0;
static HMODULE myHDLL;
static HHOOK handle = 0;
static HHOOK handle1 = 0;
static HHOOK kb_handle = 0;

bool _overlay_on(false);
bool _block_input(false);
bool _hard_block(false);
bool _reload_1_down(false);
bool _reload_modified(false);
bool _is_game(false);
bool _is_sider(false);
bool _is_edit_mode(false);
bool _priority_set(false);

wstring _overlay_header;
wchar_t _overlay_text[4096];
wchar_t _current_overlay_text[4096] = L"hello world!";
char _overlay_utf8_text[4096];
char _overlay_utf8_image_path[2048];

wchar_t module_filename[MAX_PATH];
wchar_t dll_log[MAX_PATH];
wchar_t dll_ini[MAX_PATH];
wchar_t gamepad_ini[MAX_PATH];
wchar_t sider_dir[MAX_PATH];

static int get_team_id(MATCH_INFO_STRUCT *mi, int home_or_away);

static BYTE* get_uniparam()
{
    if (_uniparam_base) {
        // follow a few indirections
        void *obj1 = *(void**)(_uniparam_base);
        if (obj1) {
            void *obj2 = *(void**)((BYTE*)obj1+8);
            if (obj2) {
                void *obj3 = *(void**)((BYTE*)obj2+0x50);
                if (obj3) {
                    return *(BYTE**)((BYTE*)obj3+8);
                }
            }
        }
    }
    return NULL;
}

static char *suffix_map[] = {
    "1st",  "2nd", "3rd", "4th", "5th",
    "6th",  "7th", "8th", "9th", "10th",
};
static char *gk_suffix_map[] = {
    "GK1st",
};

static BYTE* find_kit_info(int team_id, char *suffix, DWORD *len=NULL)
{
    BYTE *uniparam = get_uniparam();
    if (uniparam) {
        DWORD numItems = *(DWORD*)(uniparam);
        DWORD sec3start = *(DWORD*)(uniparam+4);
        DWORD sec3end = sec3start + numItems * 0x0c;
        for (DWORD offs = sec3start; offs != sec3end; offs += 0x0c ) {
            DWORD cf_starting_offs = *(DWORD*)(uniparam+offs);
            DWORD cf_len = *(DWORD*)(uniparam+offs+4);
            DWORD cf_name_starting_offs = *(DWORD*)(uniparam+offs+8);

            char *kit_config_name = strdup((char*)uniparam + cf_name_starting_offs);

            char *first_underscore = strchr(kit_config_name,'_');
            if (first_underscore) {
                char *second_underscore = strchr(first_underscore+1,'_');

                *first_underscore = '\0';
                int id = 0;
                if (sscanf(kit_config_name,"%d",&id)==1) {
                    if ((team_id == id && memcmp(first_underscore+1, "DEF", 3)==0) ||
                        (team_id == id + 0x10000 && memcmp(first_underscore+1, "ACL", 3)==0)) {
                        if (memcmp(second_underscore+1, suffix, strlen(suffix))==0) {
                            BYTE *p = uniparam + cf_starting_offs;
                            char *n = (char*)uniparam + cf_name_starting_offs;
                            size_t n_len = strlen(n);
                            size_t suff_len = strlen("_realUni.bin");
                            if ((n_len > suff_len) && memcmp(n + n_len - suff_len, "_realUni.bin", suff_len)==0) {
                                // matched a licensed kit
                                logu_("find_kit_info:: name: {%s}\n", (char*)uniparam + cf_name_starting_offs);
                                free(kit_config_name);
                                if (len) {
                                    *len = cf_len;
                                }
                                return p;
                            }
                        }
                    }
                }
            }
            free(kit_config_name);
        }
    }
    return NULL;
}

class hook_cache_t {
public:
    wstring _filename;
    void* _addrs[64];
    int _len;
    hook_cache_t(const wstring &filename) : _filename(filename), _len(0) {
        memset(_addrs, 0, sizeof(_addrs));
        FILE *f = _wfopen(_filename.c_str(), L"rb");
        if (f) {
            logu_("hook cache: loading...\n");
            while (!feof(f)) {
                void *addr;
                if (fread(&addr, sizeof(void*), 1, f)) {
                    _addrs[_len++] = addr;
                }
            }
            fclose(f);
            logu_("hook cache: read %d entries\n", _len);
        }
    }
    void save() {
        FILE *f = _wfopen(_filename.c_str(), L"wb");
        if (!f) {
            logu_("warning: unable to save hook cache to: %s\n", _filename.c_str());
            return;
        }
        logu_("hook cache: saving...\n");
        for (int i=0; i<_len; i++) {
            fwrite(&_addrs[i], sizeof(void*), 1, f);
        }
        logu_("hook cache: written %d entries\n", _len);
        fclose(f);
    }
    void* get(int i) {
        if (0 <= i && i < sizeof(_addrs)/sizeof(void*)) {
            return _addrs[i];
        }
        return NULL;
    }
    void set(int i, void* p) {
        if (0 <= i && i < sizeof(_addrs)/sizeof(void*)) {
            _addrs[i] = p;
            if (i >= _len) {
                _len = i+1;
            }
        }
    }
};

struct gamepad_directinput_mapping_t {
    DWORD dwType;
    int what;
};

struct gamepad_vkey_mapping_t {
    int what;
    int value;
    BYTE vkey;
};

namespace GAMEPAD {
    const int A = 0;
    const int B = 1;
    const int X = 2;
    const int Y = 3;
    const int LB = 4;
    const int RB = 5;
    const int START = 6;
    const int BACK = 7;
    const int LS = 8;
    const int RS = 9;
    const int LT = 10;
    const int RT = 11;
    const int LSx = 12;
    const int LSy = 13;
    const int RSx = 14;
    const int RSy = 15;
    const int DPAD = 16;
};

wchar_t *_xi_names[] = {
    L"A", L"B", L"X", L"Y", L"LB",
    L"RB", L"START", L"BACK", L"LS", L"RS",
    L"LT", L"RT",
    L"LSx", L"LSy", L"RSx", L"RSy",
    L"DPAD"
};

char *_xi_utf8_names[] = {
    "A", "B", "X", "Y", "LB",
    "RB", "START", "BACK", "LS", "RS",
    "LT", "RT",
    "LSx", "LSy", "RSx", "RSy",
    "DPAD"
};

int _pov_map[] = {
    0,45,90,135,180,225,270,315,
    1,9,8,10,2,6,4,5,
};
int _fast_pov_map[512];

int xi_name_to_number(const wchar_t *s) {
    for (int i=0; i<17; i++) {
        if (wcscmp(s, _xi_names[i])==0) {
            return i;
        }
    }
    return -1;
}

void init_fast_pov_map() {
    memset(_fast_pov_map, 0, sizeof(_fast_pov_map));
    for (int i=0; i<8; i++) {
        _fast_pov_map[_pov_map[i]] = _pov_map[i+8];
    }
}

class gamepad_config_t {
public:
    wstring _section_name;
    unordered_map<DWORD,gamepad_directinput_mapping_t> _di_map;
    unordered_map<uint64_t,gamepad_vkey_mapping_t> _vkey_map;
    bool _dinput_enabled;
    bool _xinput_enabled;
    int _overlay_toggle_1;
    int _overlay_toggle_2;
    int _overlay_next_module;
    int _overlay_prev_module;
    float _stick_sensitivity;
    DWORD _gamepad_poll_interval_msec;
    DWORD _gamepad_overlay_poll_interval_msec;

    ~gamepad_config_t() {}
    gamepad_config_t(const wstring& section_name, const wchar_t* gamepad_ini) :
            _dinput_enabled(false),
            _xinput_enabled(false),
            _overlay_toggle_1(GAMEPAD::RT),
            _overlay_toggle_2(GAMEPAD::LT),
            _overlay_next_module(GAMEPAD::LT),
            _overlay_prev_module(GAMEPAD::RT),
            _stick_sensitivity(DEFAULT_GAMEPAD_STICK_SENSITIVITY),
            _gamepad_poll_interval_msec(DEFAULT_GAMEPAD_POLL_INTERVAL_MSEC),
            _gamepad_overlay_poll_interval_msec(DEFAULT_GAMEPAD_OVERLAY_POLL_INTERVAL_MSEC),
            _section_name(section_name)
    {
        wchar_t settings[32767];
        RtlZeroMemory(settings, sizeof(settings));
        if (GetPrivateProfileSection(_section_name.c_str(),
            settings, sizeof(settings)/sizeof(wchar_t), gamepad_ini)==0) {
            // no ini-file, or no "[gamepad]" section
            return;
        }

        wchar_t* p = settings;
        while (*p) {
            wstring pair(p);
            wstring key(pair.substr(0, pair.find(L"=")));
            wstring value(pair.substr(pair.find(L"=")+1));
            string_strip_quotes(value);

            if (wcscmp(L"directinput.map", key.c_str())==0) {
                gamepad_directinput_mapping_t gdim;
                wchar_t name[128];
                memset(name,0,sizeof(name));
                if (swscanf(value.c_str(), L"%x,%s", &gdim.dwType, name)==2) {
                    gdim.what = xi_name_to_number(name);
                    if (gdim.what >= 0) {
                        _di_map.insert(std::pair<DWORD,gamepad_directinput_mapping_t>(gdim.dwType,gdim));
                    }
                }
            }
            else if (wcscmp(L"gamepad.keyboard.mapping", key.c_str())==0) {
                gamepad_vkey_mapping_t gkm;
                wstring name(L"");
                int first_comma_pos = value.find(L',');
                if (first_comma_pos != string::npos) {
                    name = value.substr(0, first_comma_pos);
                    //log_(L"name = {%s}\n", name);
                    value = value.substr(first_comma_pos + 1);
                    if (swscanf(value.c_str(), L"%d,%hhx", &gkm.value, &gkm.vkey)==2) {
                        gkm.what = xi_name_to_number(name.c_str());
                        if (gkm.what >= 0) {
                            uint64_t key = (((uint64_t)gkm.what << 32) & 0xffffffff00000000L ) | ((uint64_t)gkm.value & 0x00000000ffffffffL);
                            _vkey_map.insert(std::pair<uint64_t,gamepad_vkey_mapping_t>(key,gkm));
                        }
                    }
                }
            }
            else if (wcscmp(L"gamepad.overlay.toggle-1", key.c_str())==0) {
                int what = xi_name_to_number(value.c_str());
                if (what != -1) {
                    _overlay_toggle_1 = what;
                }
            }
            else if (wcscmp(L"gamepad.overlay.toggle-2", key.c_str())==0) {
                int what = xi_name_to_number(value.c_str());
                if (what != -1) {
                    _overlay_toggle_2 = what;
                }
            }
            else if (wcscmp(L"gamepad.overlay.next-module", key.c_str())==0) {
                int what = xi_name_to_number(value.c_str());
                if (what != -1) {
                    _overlay_next_module = what;
                }
            }
            else if (wcscmp(L"gamepad.overlay.prev-module", key.c_str())==0) {
                int what = xi_name_to_number(value.c_str());
                if (what != -1) {
                    _overlay_prev_module = what;
                }
            }
            else if (wcscmp(L"gamepad.stick-sensitivity", key.c_str())==0) {
                float v = _stick_sensitivity;
                if (swscanf(value.c_str(),L"%f",&v)==1) {
                    _stick_sensitivity = min(0.95f, max(0.35f, v));
                }
            }

            p += wcslen(p) + 1;
        }

        _gamepad_overlay_poll_interval_msec = GetPrivateProfileInt(_section_name.c_str(),
            L"gamepad.overlay.poll-interval-msec", _gamepad_overlay_poll_interval_msec,
            gamepad_ini);

        _gamepad_poll_interval_msec = GetPrivateProfileInt(_section_name.c_str(),
            L"gamepad.poll-interval-msec", _gamepad_poll_interval_msec,
            gamepad_ini);

        _dinput_enabled = GetPrivateProfileInt(_section_name.c_str(),
            L"gamepad.dinput.enabled", _dinput_enabled,
            gamepad_ini);

        _xinput_enabled = GetPrivateProfileInt(_section_name.c_str(),
            L"gamepad.xinput.enabled", _xinput_enabled,
            gamepad_ini);

    }

    bool lookup_di(DWORD dwType, int *what) {
        unordered_map<DWORD,gamepad_directinput_mapping_t>::iterator it;
        it = _di_map.find(dwType);
        if (it != _di_map.end()) {
            *what = it->second.what;
            return true;
        }
        return false;
    }

    bool lookup(int what, int value, BYTE* vkey) {
        uint64_t key = (((uint64_t)what << 32) & 0xffffffff00000000L ) | ((uint64_t)value & 0x00000000ffffffffL);
        unordered_map<uint64_t,gamepad_vkey_mapping_t>::iterator it;
        it = _vkey_map.find(key);
        if (it != _vkey_map.end()) {
            *vkey = it->second.vkey;
            return true;
        }
        return false;
    }
};

config_t* _config;
gamepad_config_t* _gamepad_config;

typedef struct {
    void *value;
    uint64_t expires;
} cache_map_value_t;

/*
GetTickCount64 is pretty fast, almost as fast as GetTickCount, but does not
have a problem of wrap-around every 49 days. So we use it.

GetTickCount64:

00007FF818F261B0 | 8B 0C 25 04 00 FE 7F                 | mov ecx,dword ptr ds:[7FFE0004]  |
00007FF818F261B7 | B8 20 03 FE 7F                       | mov eax,7FFE0320                 |
00007FF818F261BC | 48 C1 E1 20                          | shl rcx,20                       |
00007FF818F261C0 | 48 8B 00                             | mov rax,qword ptr ds:[rax]       |
00007FF818F261C3 | 48 C1 E0 08                          | shl rax,8                        |
00007FF818F261C7 | 48 F7 E1                             | mul rcx                          |
00007FF818F261CA | 48 8B C2                             | mov rax,rdx                      |
00007FF818F261CD | C3                                   | ret                              |
*/

class stats_t {
public:
    uint64_t _total_count;
    uint64_t _total;
    uint64_t _min;
    uint64_t _max;
    wstring _name;

    stats_t(const wchar_t *name) : _name(name), _total_count(0), _total(0), _min((uint64_t)-1), _max(0) {}
    ~stats_t() {
        log_(L"stats (%s): total_count:%d\n", _name.c_str(), _total_count);
        log_(L"stats (%s): total:%d\n", _name.c_str(), _total);
        log_(L"stats (%s): min:%d\n", _name.c_str(), _min);
        log_(L"stats (%s): max:%d\n", _name.c_str(), _max);
        if (_total_count > 0) {
            double avg = (double)_total / _total_count;
            log_(L"stats (%s): avg:%0.3f\n", _name.c_str(), avg);
        }
    }
};

class perf_timer_t {
public:
    stats_t *_stats;
    uint64_t _s, _e;
    perf_timer_t(stats_t *stats) : _stats(stats) {
        QueryPerformanceCounter((LARGE_INTEGER*)&_s);
    }
    ~perf_timer_t() {
        QueryPerformanceCounter((LARGE_INTEGER*)&_e);
        uint64_t elapsed = _e - _s;
        _stats->_total_count++;
        _stats->_total += elapsed;
        if (_stats->_min > elapsed) {
            _stats->_min = elapsed;
        }
        if (_stats->_max < elapsed) {
            _stats->_max = elapsed;
        }
    }
};

stats_t *_stats(NULL);
stats_t *_content_stats(NULL);
stats_t *_fileops_stats(NULL);
stats_t *_overlay_stats(NULL);

#ifdef PERF_TESTING
#define PERF_TIMER(stats) perf_timer_t timer(stats)
#else
#define PERF_TIMER(stats)
#endif

#define CACHE2_KEY_LEN 512
struct cache2_value_t {
    char key[CACHE2_KEY_LEN];
    void *value;
    uint64_t expires;
};
class cache2_t {
public:
    cache2_value_t *_data;
    size_t _data_count;
    size_t _max_idx;
    size_t _next;
    CRITICAL_SECTION _cs;
    uint64_t _ttl_msec;
    stats_t *_lookup_stats;
    stats_t *_put_stats;

    cache2_t(size_t data_count, uint64_t ttl_sec) :
            _data_count(data_count), _max_idx(data_count-1), _ttl_msec(ttl_sec * 1000), _next(0),
            _lookup_stats(0), _put_stats(0) {
        InitializeCriticalSection(&_cs);
        _data = (cache2_value_t*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, _data_count*sizeof(cache2_value_t));
#ifdef PERF_TESTING
        _lookup_stats = new stats_t(L"lookups");
        _put_stats = new stats_t(L"puts");
#endif
    }
    ~cache2_t() {
        log_(L"cache: size:%d\n", size());
        if (_lookup_stats) { delete _lookup_stats; }
        if (_put_stats) { delete _put_stats; }
        DeleteCriticalSection(&_cs);
        HeapFree(GetProcessHeap(), 0, _data);
    }
    inline void store(size_t i, const char *key, void *value) {
        _data[i].key[0] = '\0';
        strncat(_data[i].key, key, CACHE2_KEY_LEN-1);
        _data[i].value = value;
        _data[i].expires = GetTickCount64() + _ttl_msec;
    }
    bool lookup(const char *key, void **value) {
        lock_t lock(&_cs);
        PERF_TIMER(_lookup_stats);
        uint64_t ltime = GetTickCount64();
        size_t i = _next;
        do {
            i = (i - 1) & _max_idx;
            if (_data[i].expires > ltime) {
                if (strncmp(_data[i].key, key, CACHE2_KEY_LEN)==0) {
                    *value = _data[i].value;
                    return true;
                }
            }
        }
        while (i != _next);
        return false;
    }
    void put(const char *key, void *value) {
        lock_t lock(&_cs);
        PERF_TIMER(_put_stats);
        store(_next, key, value);
        _next = (_next + 1) & _max_idx;
    }
    size_t size() {
        return _data_count;
    }
};

typedef unordered_map<string,cache_map_value_t> cache_map_t;

class cache_t {
    cache_map_t _map;
    uint64_t _ttl_msec;
    CRITICAL_SECTION _kcs;
    int debug;
    stats_t *_lookup_stats;
    stats_t *_put_stats;
public:
    cache_t(int ttl_sec) :
        _lookup_stats(NULL), _put_stats(NULL), _ttl_msec(ttl_sec * 1000) {
        InitializeCriticalSection(&_kcs);
#ifdef PERF_TESTING
        _lookup_stats = new stats_t(L"lookups");
        _put_stats = new stats_t(L"puts");
#endif
    }
    ~cache_t() {
        log_(L"cache: size:%d\n", _map.size());
        if (_lookup_stats) { delete _lookup_stats; }
        if (_put_stats) { delete _put_stats; }
        DeleteCriticalSection(&_kcs);
    }
    bool lookup(const char *filename, void **res) {
        lock_t lock(&_kcs);
        PERF_TIMER(_lookup_stats);
        cache_map_t::iterator i = _map.find(filename);
        if (i != _map.end()) {
            uint64_t ltime = GetTickCount64();
            //logu_("key_cache::lookup: %s %llu > %llu\n", filename, i->second.expires, ltime);
            if (i->second.expires > ltime) {
                // hit
                *res = i->second.value;
                //logu_("lookup FOUND: (%08x) %s\n", i->first, filename);
                return true;
            }
            else {
                // hit, but expired value, so: miss
                DBG(32) logu_("lookup EXPIRED MATCH: (%p|%llu <= %llu) %s\n", i->second.value, i->second.expires, ltime, filename);
                _map.erase(i);
            }
        }
        else {
            // miss
        }
        *res = NULL;
        return false;
    }
    void put(const char *filename, void *value) {
        PERF_TIMER(_put_stats);
        uint64_t ltime = GetTickCount64();
        cache_map_value_t v;
        v.value = value;
        v.expires = ltime + _ttl_msec;
        DBG(32) logu_("cache::put: %s --> (%p|%llu)\n", (filename)?filename:"(NULL)", v.value, v.expires);
        {
            lock_t lock(&_kcs);
            pair<cache_map_t::iterator,bool> res = _map.insert(
                pair<string,cache_map_value_t>(filename, v));
            if (!res.second) {
                // replace existing
                //logu_("REPLACED for: %s\n", filename);
                res.first->second.value = v.value;
                res.first->second.expires = v.expires;
            }
        }
    }
    size_t size() {
        return _map.size();
    }
};

//cache_t *_key_cache(NULL);
cache2_t *_small_key_cache(NULL);

static BYTE* dummify_uniparam(BYTE *uniparam, size_t sz, size_t *new_sz)
{
    map<string,uniparam_info_t> uni;
    map<int,uniparam_team_t> teams;

    char *strips[] = {"1st", "2nd", "GK1st"};

    // read existing data structure
    logu_("uniparam:: step 1: reading existing structure...\n");

    DWORD numItems = *(DWORD*)(uniparam);
    logu_("numItems = %08x\n", numItems);
    DWORD sec3start = *(DWORD*)(uniparam+4);
    DWORD sec3end = sec3start + numItems * 0x0c;
    for (DWORD offs = sec3start; offs != sec3end; offs += 0x0c ) {
        DWORD cf_starting_offs = *(DWORD*)(uniparam+offs);
        DWORD cf_len = *(DWORD*)(uniparam+offs+4);
        DWORD cf_name_starting_offs = *(DWORD*)(uniparam+offs+8);

        char *kit_config_name = strdup((char*)uniparam + cf_name_starting_offs);

        char *first_underscore = strchr(kit_config_name,'_');
        if (first_underscore) {
            char *second_underscore = strchr(first_underscore+1,'_');
            *first_underscore = ' '; // replace with space for easy scanf

            int team_id = 0;
            if (sscanf(kit_config_name,"%d",&team_id)==1) {
                //logu_("processing kit for team %d:: %s\n", team_id, (char*)uniparam + cf_name_starting_offs);
            }
            else {
                //logu_("processing kit :: %s\n", (char*)uniparam + cf_name_starting_offs);
                team_id = 0x10000000; // fake id: for referees
            }

            uniparam_info_t entry;
            entry.block = uniparam + cf_starting_offs;
            entry.size = cf_len;
            entry.name = strdup((char*)uniparam + cf_name_starting_offs);
            string key(kit_config_name);  // use the name with space to ensure Konami-specific sort order
            uni.insert(pair<string,uniparam_info_t>(key, entry));

            if (team_id == 0x10000000) {
                continue;
            }

            map<int,uniparam_team_t>::iterator it = teams.find(team_id);
            if (it == teams.end()) {
                // add new team entry
                uniparam_team_t te;
                te.has_1st = false;
                te.has_2nd = false;
                te.has_gk1st = false;

                pair<map<int,uniparam_team_t>::iterator, bool> res;
                res = teams.insert(pair<int,uniparam_team_t>(team_id, te));
                it = res.first;
            }

            // check for specific kits and set flags
            if (second_underscore && it != teams.end()) {
                char *n = (char*)uniparam + cf_name_starting_offs;
                BYTE *p = uniparam + cf_starting_offs;

                bool *flags[3];
                flags[0] = &(it->second.has_1st);
                flags[1] = &(it->second.has_2nd);
                flags[2] = &(it->second.has_gk1st);

                size_t n_len = strlen(n);
                size_t suffix_len = strlen("realUni.bin");
                for (int i=0; i<3; i++) {
                    if (memcmp(second_underscore+1, strips[i], strlen(strips[i]))==0) {
                        if (n_len > suffix_len) {
                            char *s = n + n_len - suffix_len;
                            if (memcmp(s, "realUni.bin", suffix_len)==0) {
                                // found real uni
                                bool *flag = flags[i];
                                *flag = true;
                                //logu_("found %s real uni for %d\n", strips[i], team_id);
                            }
                        }
                    }
                }
            }
        }

        free(kit_config_name);
    }

    logu_("processed %d kits\n", uni.size());
    logu_("processed %d teams\n", teams.size());
    logu_("uniparam:: step 2: starting the dummification ...\n");

    BYTE dummy_block[0x80];
    char dummy_name[0x80];
    char dummy_key[0x80];
    memset(dummy_block, 0, sizeof(dummy_block));

    int dummy_count = 0;
    map<int,uniparam_team_t>::iterator j;
    for (j = teams.begin(); j != teams.end(); j++) {
        if (j->first == 0x10000000) {
            // fake id for referees
            continue;
        }

        bool flags[3];
        flags[0] = j->second.has_1st;
        flags[1] = j->second.has_2nd;
        flags[2] = j->second.has_gk1st;

        // add dummies
        for (int k=0; k<3; k++) {
            if (!flags[k]) {
                sprintf(dummy_name, "%d_DEF_%s_realUni.bin", j->first, strips[k]);
                sprintf(dummy_key, "%d DEF_%s_realUni.bin", j->first, strips[k]); // with a space for correct sorting

                uniparam_info_t entry;
                entry.block = dummy_block;
                entry.size = 0x78;
                entry.name = strdup(dummy_name);

                string key(dummy_key);
                uni.insert(pair<string,uniparam_info_t>(key, entry));
                //logu_("added dummy kit: %s\n", entry.name);
                dummy_count++;
            }
        }
    }

    logu_("uniparam:: added %d dummy kit blocks\n", dummy_count);
    logu_("uniparam:: step 3: rebuild the structure\n");

    // 1st pass: count the memory required
    size_t mem_req = 8; // header
    int kit_count = uni.size();
    // add toc size
    mem_req += kit_count * 0xc;
    // names
    map<string,uniparam_info_t>::iterator it;
    for (it = uni.begin(); it != uni.end(); it++) {
        mem_req += strlen(it->second.name)+1;
    }
    // align at 0x10
    mem_req += ((mem_req % 0x10) > 0) ? 0x10 - (mem_req % 0x10) : 0;
    // blocks
    for (it = uni.begin(); it != uni.end(); it++) {
        size_t len = it->second.size;
        mem_req += len;
        mem_req += ((len % 0x10) > 0) ? 0x10 - (len % 0x10) : 0; // adjust for alignment
    }
    logu_("memory needed: %d bytes\n", mem_req);
    logu_("total kits in new structure: %d\n", kit_count);

    // 2nd pass: allocate and fill with data
    BYTE *new_uni = (BYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, mem_req + 0x10);
    *(size_t*)(new_uni+8) = mem_req; // record size;
    BYTE *new_mem = new_uni + 0x10; // actual data starts here
    if (!new_mem) {
        logu_("uniparam:: FATAL problem: unable to allocate memory for new structure\n");
        *new_sz = 0;
        return NULL;
    }

    *(DWORD*)new_mem = kit_count;
    *(DWORD*)(new_mem+4) = 8;
    DWORD toc_offs = 8;
    DWORD offs = 8 + kit_count*0xc;
    // names
    for (it = uni.begin(); it != uni.end(); it++) {
        size_t len = strlen(it->second.name)+1; // with zero-terminator
        memcpy((char*)new_mem + offs, it->second.name, len);
        // update toc
        *(DWORD*)(new_mem + toc_offs + 8) = offs;
        // next
        offs += len;
        toc_offs += 0xc;
    }
    // align at 0x10
    offs += ((offs % 0x10) > 0) ? 0x10 - (offs % 0x10) : 0;
    // blocks
    toc_offs = 8;
    for (it = uni.begin(); it != uni.end(); it++) {
        size_t len = it->second.size;
        memcpy((char*)new_mem + offs, it->second.block, len);
        // update toc
        *(DWORD*)(new_mem + toc_offs) = offs;
        *(DWORD*)(new_mem + toc_offs + 4) = len;
        // next
        offs += len;
        offs += ((len % 0x10) > 0) ? 0x10 - (len % 0x10) : 0; // adjust for alignment
        toc_offs += 0xc;
    }

    logu_("uniparam:: new structure created at: %p\n", new_mem);

    // free temp memory
    for (it = uni.begin(); it != uni.end(); it++) {
        if (it->second.name) {
            free(it->second.name);
        }
    }

    // DEBUG:: dump to disk
    DBG(4096) {
        wstring fname(sider_dir);
        fname += L"new_uni.bin";
        FILE *f = _wfopen(fname.c_str(), L"wb");
        fwrite(new_mem, mem_req, 1, f);
        fclose(f);
    }
    DBG(4096) {
        wstring fname(sider_dir);
        fname += L"old_uni.bin";
        FILE *f = _wfopen(fname.c_str(), L"wb");
        fwrite(uniparam, sz, 1, f);
        fclose(f);
    }

    *new_sz = mem_req;
    return new_mem;
}

// optimization: count for all registered handlers of "livecpk_rewrite" event
// if it is 0, then no need to call do_rewrite
int _rewrite_count(0);

struct module_t {
    //lookup_cache_t *cache;
    lua_State* L;
    wstring *filename;
    FILETIME last_modified;
    int stack_position;
    int evt_trophy_check;
    int evt_lcpk_make_key;
    int evt_lcpk_get_filepath;
    int evt_lcpk_rewrite;
    int evt_lcpk_read;
    int evt_lcpk_data_ready;
    int evt_set_teams;
    int evt_set_kits;
    int evt_set_home_team_for_kits;
    int evt_set_away_team_for_kits;
    /*
    int evt_set_tid;
    */
    int evt_set_match_time;
    int evt_set_stadium_choice;
    int evt_set_stadium;
    int evt_set_conditions;
    int evt_set_match_settings;
    int evt_after_set_conditions;
    /*
    int evt_set_stadium_for_replay;
    int evt_set_conditions_for_replay;
    int evt_after_set_conditions_for_replay;
    */
    int evt_get_ball_name;
    int evt_get_stadium_name;
    /*
    int evt_enter_edit_mode;
    int evt_exit_edit_mode;
    int evt_enter_replay_gallery;
    int evt_exit_replay_gallery;
    */
    int evt_overlay_on;
    int evt_key_down;
    int evt_key_up;
    int evt_gamepad_input;
    int evt_show;
    int evt_hide;
    int evt_context_reset;
    int evt_custom;
    int evt_display_frame;
};
vector<module_t*> _modules;
module_t* _curr_m;
vector<module_t*>::iterator _curr_overlay_m;

static void push_env_table(lua_State *L, module_t* m);

bool init_paths() {
    wchar_t *p;

    // prep log filename
    memset(dll_log, 0, sizeof(dll_log));
    if (GetModuleFileName(myHDLL, dll_log, MAX_PATH)==0) {
        return FALSE;
    }
    p = wcsrchr(dll_log, L'.');
    wcscpy(p, L".log");

    // prep ini filename
    memset(dll_ini, 0, sizeof(dll_ini));
    wcscpy(dll_ini, dll_log);
    p = wcsrchr(dll_ini, L'.');
    wcscpy(p, L".ini");

    // prep gamepad.ini filename
    memset(gamepad_ini, 0, sizeof(gamepad_ini));
    wcscpy(gamepad_ini, dll_log);
    p = wcsrchr(gamepad_ini, L'\\');
    wcscpy(p, L"\\gamepad.ini");

    // prep sider dir
    memset(sider_dir, 0, sizeof(sider_dir));
    wcscpy(sider_dir, dll_log);
    p = wcsrchr(sider_dir, L'\\');
    *(p+1) = L'\0';

    return true;
}

static int sider_log(lua_State *L) {
    const char *s = luaL_checkstring(L, -1);
    lua_getfield(L, lua_upvalueindex(1), "_FILE");
    const char *fname = lua_tostring(L, -1);
    logu_("[%s] %s\n", fname, s);
    lua_pop(L, 2);
    return 0;
}

static int sider_match_get_stats(lua_State *L) {
    lua_pop(L, lua_gettop(L));
    if (_sci == NULL) {
        lua_pushnil(L);
        return 1;
    }
    //logu_("_sci = %p\n", _sci);
    lua_pushvalue(L, lua_upvalueindex(1));

    lua_pushstring(L, "ptr");
    lua_pushlightuserdata(L, _sci);
    lua_rawset(L, -3);

    int home_score = _sci->home_score;
    int away_score = _sci->away_score;
    if (_sci->sci2 != NULL) {
        // more accurate: updates sooner
        home_score = _sci->sci2->home_score;
        away_score = _sci->sci2->away_score;
    }

    lua_pushstring(L, "home_score");
    lua_pushnumber(L, home_score);
    lua_rawset(L, -3);
    lua_pushstring(L, "away_score");
    lua_pushnumber(L, away_score);
    lua_rawset(L, -3);
    if (_sci->sci2 != NULL) {
        lua_pushstring(L, "pk_home_score");
        lua_pushnumber(L, _sci->sci2->home_penalty_score);
        lua_rawset(L, -3);
        lua_pushstring(L, "pk_away_score");
        lua_pushnumber(L, _sci->sci2->away_penalty_score);
        lua_rawset(L, -3);
    }

    int period = 0;
    int minutes = _sci->clock_minutes;
    if (_sci->sci2 != NULL) {
        if (_sci->sci2->match_stage == 0x10) {
            period = 5;
        }
        else {
            if (minutes < 45) {
                period = 1;
            }
            else if (minutes < 90) {
                period = (_sci->is_injury_time) ? 1 : 2;
            }
            else if (minutes < 105) {
                period = (_sci->is_injury_time) ? 2 : 3;
            }
            else if (minutes < 120) {
                period = (_sci->is_injury_time) ? 3 : 4;
            }
            else {
                period = 4;
            }
        }
    }
    lua_pushstring(L, "period");
    lua_pushnumber(L, period);
    lua_rawset(L, -3);
    lua_pushstring(L, "clock_minutes");
    lua_pushnumber(L, minutes);
    lua_rawset(L, -3);
    lua_pushstring(L, "clock_seconds");
    lua_pushnumber(L, _sci->clock_seconds);
    lua_rawset(L, -3);
    lua_pushstring(L, "added_minutes");
    if (_sci->added_minutes != 0xffffffff) {
        lua_pushnumber(L, _sci->added_minutes);
    }
    else {
        lua_pushnil(L);
    }
    lua_rawset(L, -3);
    return 1;
}

void read_configuration(config_t*& config)
{
    config = new config_t(L"sider", dll_ini);

    // modify vertex buffer to set overlay bg-color
    DWORD v = config->_overlay_background_color;
    float r,g,b,a;
    a = (float)((v >> 24) & 0xff) / 255.0;
    b = (float)((v >> 16) & 0xff) / 255.0;
    g = (float)((v >> 8) & 0xff) / 255.0;
    r = (float)(v & 0xff) / 255.0;

    for (int i=0; i<6; i++) {
        g_vertices[i].r = r;
        g_vertices[i].g = g;
        g_vertices[i].b = b;
        g_vertices[i].a = a;
    }

    // modify constant buffer
    g_constants.maxAlpha = config->_overlay_image_alpha_max;
}

void read_gamepad_global_mapping(gamepad_config_t*& config)
{
    config = new gamepad_config_t(L"gamepad", gamepad_ini);
}

static bool skip_process(wchar_t* name)
{
    wchar_t *filename = wcsrchr(name, L'\\');
    if (filename) {
        if (wcsicmp(filename, L"\\explorer.exe") == 0) {
            return true;
        }
        if (wcsicmp(filename, L"\\steam.exe") == 0) {
            return true;
        }
        if (wcsicmp(filename, L"\\steamwebhelper.exe") == 0) {
            return true;
        }
    }
    return false;
}

static bool is_sider(wchar_t* name)
{
    wchar_t *filename = wcsrchr(name, L'\\');
    if (filename) {
        if (wcsicmp(filename, L"\\sider.exe") == 0) {
            return true;
        }
    }
    return false;
}

static bool is_already_loaded(HMODULE hDLL)
{
    HMODULE hModules[1024];
    DWORD cb_needed;
    wchar_t me[MAX_PATH];
    wchar_t t[MAX_PATH];

    GetModuleFileName(hDLL, me, MAX_PATH);

    if (EnumProcessModules(GetCurrentProcess(), hModules, sizeof(hModules), &cb_needed)) {
        log_(L"EnumProcessModules: found %d loaded DLLs\n", cb_needed/sizeof(HMODULE));
        for (int i=0; i<cb_needed/sizeof(HMODULE); i++) {
            GetModuleFileName(hModules[i], t, MAX_PATH);
            wchar_t *filename = wcsrchr(t, L'\\');
            if (filename && wcsicmp(filename, L"\\sider.dll")==0 && wcsicmp(t, me)!=0) {
                log_(L"another sider.dll already loaded from: %s\n", t);
                return true;
            }
        }
    }
    else {
        log_(L"EnumProcessModules failed with: 0x%x\n", GetLastError());
    }
    return false;
}

static bool is_pes(wchar_t* name, wstring** match)
{
    bool result = false;
    wchar_t *filename = wcsrchr(name, L'\\');
    if (filename) {
        vector<wstring>::iterator it = _config->_exe_names.begin();
        for (; it != _config->_exe_names.end(); it++) {
            if (wcsicmp(filename, it->c_str()) == 0) {
                *match = new wstring(*it);
                result = true;
                break;
            }
        }
    }
    return result;
}

void set_controller_poll_delay() {
    _controller_poll_delay = (_overlay_on) ?
        _gamepad_config->_gamepad_overlay_poll_interval_msec :
        _gamepad_config->_gamepad_poll_interval_msec;
}

BOOL sider_device_enum_callback(LPCDIDEVICEINSTANCE lppdi, LPVOID pvRef)
{
    log_(L"controller: type: %x name: %s\n", lppdi->dwDevType, lppdi->tszInstanceName);
    if (!_has_controller) {
        g_controller_guid_instance = lppdi->guidInstance;
        _has_controller = true;
    }
    return DIENUM_CONTINUE;
}

BOOL sider_object_enum_callback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
    log_(L"object: name: %s, type: 0x%x, ofs: 0x%x\n", lpddoi->tszName, lpddoi->dwType, lpddoi->dwOfs);
    DIDEVICEOBJECTINSTANCE ddoi;
    memcpy(&ddoi, lpddoi,  sizeof(ddoi));
    _di_objects.push_back(ddoi);
    int what;
    if (_gamepad_config->lookup_di(ddoi.dwType, &what)) {
        // toggle 1
        if (what == _gamepad_config->_overlay_toggle_1) {
            _di_overlay_toggle1.dwType = ddoi.dwType;
            _di_overlay_toggle1.dwOfs = ddoi.dwOfs;
            _di_overlay_toggle1.what = what;
        }
        // toggle 2
        if (what == _gamepad_config->_overlay_toggle_2) {
            _di_overlay_toggle2.dwType = ddoi.dwType;
            _di_overlay_toggle2.dwOfs = ddoi.dwOfs;
            _di_overlay_toggle2.what = what;
        }
        // next module
        if (what == _gamepad_config->_overlay_next_module) {
            _di_module_switch.dwType = ddoi.dwType;
            _di_module_switch.dwOfs = ddoi.dwOfs;
            _di_module_switch.what = what;
        }
        // prev module
        if (what == _gamepad_config->_overlay_prev_module) {
            _di_module_switch_prev.dwType = ddoi.dwType;
            _di_module_switch_prev.dwOfs = ddoi.dwOfs;
            _di_module_switch_prev.what = what;
        }
    }
    return DIENUM_CONTINUE;
}

wchar_t* _have_live_file(char *file_name)
{
    wchar_t *unicode_filename = Utf8::utf8ToUnicode(file_name);
    //wchar_t unicode_filename[512];
    //memset(unicode_filename, 0, sizeof(unicode_filename));
    //Utf8::fUtf8ToUnicode(unicode_filename, file_name);

    wchar_t fn[512];
    for (vector<wstring>::iterator it = _config->_cpk_roots.begin();
            it != _config->_cpk_roots.end();
            it++) {
        fn[0] = L'\0';
        wcsncat(fn, it->c_str(), 512);
        wchar_t *p = (unicode_filename[0] == L'\\') ? unicode_filename + 1 : unicode_filename;
        wcsncat(fn, p, 512);

        HANDLE handle;
        handle = CreateFileW(fn,           // file to open
                           GENERIC_READ,          // open for reading
                           FILE_SHARE_READ,       // share for reading
                           NULL,                  // default security
                           OPEN_EXISTING,         // existing file only
                           FILE_ATTRIBUTE_NORMAL,  // normal file
                           NULL);                 // no attr. template

        if (handle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(handle);
            Utf8::free(unicode_filename);
            //return new wstring(fn);
            return wcsdup(fn);
        }
    }

    Utf8::free(unicode_filename);
    return NULL;
}

wchar_t* have_live_file(char *file_name)
{
    PERF_TIMER(_stats);
    return _have_live_file(file_name);
}

bool file_exists(wchar_t *fullpath, LONGLONG *size)
{
    HANDLE handle = CreateFileW(
        fullpath,     // file to open
        GENERIC_READ,          // open for reading
        FILE_SHARE_READ,       // share for reading
        NULL,                  // default security
        OPEN_EXISTING,         // existing file only
        FILE_ATTRIBUTE_NORMAL,  // normal file
        NULL);                 // no attr. template

    if (handle != INVALID_HANDLE_VALUE)
    {
        if (size != NULL) {
            DWORD *p = (DWORD*)size;
            *size = GetFileSize(handle, p+1);
        }
        CloseHandle(handle);
        return true;
    }
    return false;
}

void clear_context_fields(const char **names, size_t num_items)
{
    if (_config->_lua_enabled) {
        EnterCriticalSection(&_cs);
        lua_pushvalue(L, 1); // ctx
        for (int i=0; i<num_items; i++) {
            lua_pushnil(L);
            lua_setfield(L, -2, names[i]);
        }
        lua_pop(L, 1);
        LeaveCriticalSection(&_cs);
    }
}

int get_context_field_int(const char *name, int default_value)
{
    int value = default_value;
    if (_config->_lua_enabled) {
        EnterCriticalSection(&_cs);
        lua_pushvalue(L, 1); // ctx
        lua_getfield(L, -1, name);
        if (lua_isnumber(L, -1)) {
            value = luaL_checkinteger(L, -1);
        }
        lua_pop(L, 2);
        LeaveCriticalSection(&_cs);
    }
    return value;
}

void set_context_field_lightuserdata(const char *name, void *p)
{
    if (_config->_lua_enabled) {
        EnterCriticalSection(&_cs);
        lua_pushvalue(L, 1); // ctx
        lua_pushlightuserdata(L, p);
        lua_setfield(L, -2, name);
        lua_pop(L, 1);
        LeaveCriticalSection(&_cs);
    }
}

void set_context_field_int(const char *name, int value)
{
    if (_config->_lua_enabled) {
        EnterCriticalSection(&_cs);
        lua_pushvalue(L, 1); // ctx
        lua_pushinteger(L, value);
        lua_setfield(L, -2, name);
        lua_pop(L, 1);
        LeaveCriticalSection(&_cs);
    }
}

void set_context_field_nil(const char *name)
{
    if (_config->_lua_enabled) {
        EnterCriticalSection(&_cs);
        lua_pushvalue(L, 1); // ctx
        lua_pushnil(L);
        lua_setfield(L, -2, name);
        lua_pop(L, 1);
        LeaveCriticalSection(&_cs);
    }
}

void set_context_field_boolean(const char *name, bool value)
{
    if (_config->_lua_enabled) {
        EnterCriticalSection(&_cs);
        lua_pushvalue(L, 1); // ctx
        lua_pushboolean(L, (value)?1:0);
        lua_setfield(L, -2, name);
        lua_pop(L, 1);
        LeaveCriticalSection(&_cs);
    }
}

void set_match_info(MATCH_INFO_STRUCT* mis)
{
    int match_id = (int)(mis->match_id);
    int match_leg = (int)(mis->match_leg);
    int match_info = (int)(mis->match_info);

    _mi = mis;
    set_context_field_lightuserdata("mis", mis);

    if (match_id != 0 && (match_leg == 0 || match_leg == 1)) {
        set_context_field_int("match_leg", match_leg+1);
    }
    else {
        set_context_field_nil("match_leg");
    }
    set_context_field_int("match_id", match_id);
    if (match_info < 128) {
        set_context_field_int("match_info", match_info);
    }
}

bool module_trophy_rewrite(module_t *m, WORD tournament_id, WORD *new_tid)
{
    *new_tid = tournament_id;
    bool assigned(false);
    if (m->evt_trophy_check != 0) {
        EnterCriticalSection(&_cs);
        lua_pushvalue(m->L, m->evt_trophy_check);
        lua_xmove(m->L, L, 1);
        // push params
        lua_pushvalue(L, 1); // ctx
        lua_pushinteger(L, tournament_id);
        if (lua_pcall(L, 2, 1, 0) != LUA_OK) {
            const char *err = luaL_checkstring(L, -1);
            logu_("[%d] lua ERROR from module_trophy_rewrite: %s\n",
                GetCurrentThreadId(), err);
        }
        else if (lua_isnumber(L, -1)) {
            *new_tid = (WORD)luaL_checkint(L, -1);
            assigned = true;
        }
        lua_pop(L, 1);
        LeaveCriticalSection(&_cs);
    }
    return assigned;
}

void module_call_callback_with_context(lua_State *L, lua_State *from_L, int callback_index) {
    EnterCriticalSection(&_cs);
    lua_pushvalue(from_L, callback_index);
    lua_xmove(from_L, L, 1);
    lua_pushvalue(L, 1); // ctx
    if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
        const char *err = luaL_checkstring(L, -1);
        logu_("[%d] lua ERROR from module_call_callback_with_context: %s\n",
            GetCurrentThreadId(), err);
    }
    LeaveCriticalSection(&_cs);
}

bool module_set_match_time(module_t *m, DWORD *num_minutes)
{
    bool res(false);
    if (m->evt_set_match_time != 0) {
        EnterCriticalSection(&_cs);
        lua_pushvalue(m->L, m->evt_set_match_time);
        lua_xmove(m->L, L, 1);
        // push params
        lua_pushvalue(L, 1); // ctx
        lua_pushinteger(L, *num_minutes);
        if (lua_pcall(L, 2, 1, 0) != LUA_OK) {
            const char *err = luaL_checkstring(L, -1);
            logu_("[%d] lua ERROR from module_set_match_time: %s\n", GetCurrentThreadId(), err);
        }
        else if (lua_isnumber(L, -1)) {
            int value = luaL_checkinteger(L, -1);
            *num_minutes = value;
            res = true;
        }
        lua_pop(L, 1);
        LeaveCriticalSection(&_cs);
    }
    return res;
}

bool module_set_stadium_choice(module_t *m, WORD stadium_id, WORD *new_stadium_id)
{
    bool res(false);
    if (m->evt_set_stadium_choice != 0) {
        EnterCriticalSection(&_cs);
        lua_pushvalue(m->L, m->evt_set_stadium_choice);
        lua_xmove(m->L, L, 1);
        // push params
        lua_pushvalue(L, 1); // ctx
        lua_pushinteger(L, stadium_id);
        if (lua_pcall(L, 2, 1, 0) != LUA_OK) {
            const char *err = luaL_checkstring(L, -1);
            logu_("[%d] lua ERROR from module_set_stadium_choice: %s\n", GetCurrentThreadId(), err);
        }
        else if (lua_isnumber(L, -1)) {
            *new_stadium_id = luaL_checkint(L, -1);
            res = true;
        }
        lua_pop(L, 1);
        LeaveCriticalSection(&_cs);
    }
    return res;
}

bool module_set_stadium(module_t *m, MATCH_INFO_STRUCT *mi)
{
    bool res(false);
    if (m->evt_set_stadium != 0) {
        EnterCriticalSection(&_cs);
        STAD_STRUCT *ss = &(mi->stad);
        lua_pushvalue(m->L, m->evt_set_stadium);
        lua_xmove(m->L, L, 1);
        // push params
        lua_pushvalue(L, 1); // ctx
        lua_newtable(L);
        lua_pushinteger(L, ss->stadium);
        lua_setfield(L, -2, "stadium");
        lua_pushinteger(L, ss->timeofday);
        lua_setfield(L, -2, "timeofday");
        lua_pushinteger(L, ss->weather);
        lua_setfield(L, -2, "weather");
        lua_pushinteger(L, mi->weather_effects);
        lua_setfield(L, -2, "weather_effects");
        lua_pushinteger(L, ss->season);
        lua_setfield(L, -2, "season");
        if (lua_pcall(L, 2, 1, 0) != LUA_OK) {
            const char *err = luaL_checkstring(L, -1);
            logu_("[%d] lua ERROR from module_set_stadium: %s\n", GetCurrentThreadId(), err);
        }
        else if (lua_istable(L, -1)) {
            lua_getfield(L, -1, "stadium");
            if (lua_isnumber(L, -1)) {
                ss->stadium = luaL_checkinteger(L, -1);
            }
            lua_pop(L, 1);
            res = true;
        }
        else if (lua_isnumber(L, -1)) {
            ss->stadium = luaL_checkinteger(L, -1);
            lua_pop(L, 1);
            res = true;
        }
        lua_pop(L, 1);
        LeaveCriticalSection(&_cs);
    }
    return res;
}

bool module_set_match_settings(module_t *m, MATCH_INFO_STRUCT *mi)
{
    bool res(false);
    if (m->evt_set_match_settings != 0) {
        EnterCriticalSection(&_cs);
        lua_pushvalue(m->L, m->evt_set_match_settings);
        lua_xmove(m->L, L, 1);
        // push params
        lua_pushvalue(L, 1); // ctx
        lua_newtable(L);
        lua_pushinteger(L, mi->difficulty);
        lua_setfield(L, -2, "difficulty");
        lua_pushinteger(L, mi->extra_time_choice);
        lua_setfield(L, -2, "extra_time");
        lua_pushinteger(L, mi->penalties);
        lua_setfield(L, -2, "penalties");
        lua_pushinteger(L, mi->num_subs);
        lua_setfield(L, -2, "substitutions");
        lua_pushinteger(L, mi->num_subs_et);
        lua_setfield(L, -2, "substitutions_in_extra_time");
        if (lua_pcall(L, 2, 1, 0) != LUA_OK) {
            const char *err = luaL_checkstring(L, -1);
            logu_("[%d] lua ERROR from module_set_match_settings: %s\n", GetCurrentThreadId(), err);
        }
        else if (lua_istable(L, -1)) {
            lua_getfield(L, -1, "difficulty");
            if (lua_isnumber(L, -1)) {
                mi->difficulty = luaL_checkinteger(L, -1);
            }
            lua_pop(L, 1);
            lua_getfield(L, -1, "extra_time");
            if (lua_isnumber(L, -1)) {
                mi->extra_time_choice = luaL_checkinteger(L, -1);
            }
            lua_pop(L, 1);
            lua_getfield(L, -1, "penalties");
            if (lua_isnumber(L, -1)) {
                mi->penalties = luaL_checkinteger(L, -1);
            }
            lua_pop(L, 1);
            lua_getfield(L, -1, "substitutions");
            if (lua_isnumber(L, -1)) {
                mi->num_subs = luaL_checkinteger(L, -1);
            }
            lua_pop(L, 1);
            lua_getfield(L, -1, "substitutions_in_extra_time");
            if (lua_isnumber(L, -1)) {
                mi->num_subs_et = luaL_checkinteger(L, -1);
            }
            lua_pop(L, 1);
            res = true;
        }
        lua_pop(L, 1);
        LeaveCriticalSection(&_cs);
    }
    return res;
}

bool module_set_conditions(module_t *m, MATCH_INFO_STRUCT *mi)
{
    bool res(false);
    if (m->evt_set_conditions != 0) {
        EnterCriticalSection(&_cs);
        STAD_STRUCT *ss = &(mi->stad);
        lua_pushvalue(m->L, m->evt_set_conditions);
        lua_xmove(m->L, L, 1);
        // push params
        lua_pushvalue(L, 1); // ctx
        lua_newtable(L);
        lua_pushinteger(L, ss->stadium);
        lua_setfield(L, -2, "stadium");
        lua_pushinteger(L, ss->timeofday);
        lua_setfield(L, -2, "timeofday");
        lua_pushinteger(L, ss->weather);
        lua_setfield(L, -2, "weather");
        lua_pushinteger(L, mi->weather_effects);
        lua_setfield(L, -2, "weather_effects");
        lua_pushinteger(L, ss->season);
        lua_setfield(L, -2, "season");
        if (lua_pcall(L, 2, 1, 0) != LUA_OK) {
            const char *err = luaL_checkstring(L, -1);
            logu_("[%d] lua ERROR from module_set_conditions: %s\n", GetCurrentThreadId(), err);
        }
        else if (lua_istable(L, -1)) {
            lua_getfield(L, -1, "timeofday");
            if (lua_isnumber(L, -1)) {
                ss->timeofday = luaL_checkinteger(L, -1);
            }
            lua_pop(L, 1);
            lua_getfield(L, -1, "weather");
            if (lua_isnumber(L, -1)) {
                ss->weather = luaL_checkinteger(L, -1);
            }
            lua_pop(L, 1);
            lua_getfield(L, -1, "weather_effects");
            if (lua_isnumber(L, -1)) {
                mi->weather_effects = luaL_checkinteger(L, -1);
            }
            lua_pop(L, 1);
            lua_getfield(L, -1, "season");
            if (lua_isnumber(L, -1)) {
                ss->season = luaL_checkinteger(L, -1);
            }
            lua_pop(L, 1);
            res = true;
        }
        lua_pop(L, 1);
        LeaveCriticalSection(&_cs);
    }
    return res;
}

void module_after_set_conditions(module_t *m)
{
    if (m->evt_after_set_conditions != 0) {
        EnterCriticalSection(&_cs);
        lua_pushvalue(m->L, m->evt_after_set_conditions);
        lua_xmove(m->L, L, 1);
        // push params
        lua_pushvalue(L, 1); // ctx
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            const char *err = luaL_checkstring(L, -1);
            logu_("[%d] lua ERROR from module_after_set_conditions: %s\n", GetCurrentThreadId(), err);
            lua_pop(L, 1);
        }
        LeaveCriticalSection(&_cs);
    }
}

void module_context_reset(module_t *m)
{
    if (m->evt_context_reset != 0) {
        EnterCriticalSection(&_cs);
        lua_pushvalue(m->L, m->evt_context_reset);
        lua_xmove(m->L, L, 1);
        // push params
        lua_pushvalue(L, 1); // ctx
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            const char *err = luaL_checkstring(L, -1);
            logu_("[%d] lua ERROR from module_context_reset: %s\n", GetCurrentThreadId(), err);
            lua_pop(L, 1);
        }
        LeaveCriticalSection(&_cs);
    }
}

bool module_custom_event(module_t *m, uint16_t param, REGISTERS *regs)
{
    bool processed = false;
    if (m->evt_custom != 0) {
        EnterCriticalSection(&_cs);
        log_(L"module_custom_event for: %s\n", m->filename->c_str());
        lua_pushvalue(m->L, m->evt_custom);
        lua_xmove(m->L, L, 1);
        // push params
        lua_pushvalue(L, 1); // ctx
        lua_pushinteger(L, param);
        lua_newtable(L); // registers
        log_(L"regs: %p\n", regs);
        registers_to_lua_table(L, -1, regs);
        log_(L"module_custom_event: registers copied to table\n");

        if (lua_pcall(L, 3, 2, 0) != LUA_OK) {
            const char *err = luaL_checkstring(L, -1);
            logu_("[%d] lua ERROR from module_custom_event: %s\n", GetCurrentThreadId(), err);
            lua_pop(L, 1);
            LeaveCriticalSection(&_cs);
            return false;
        }
        log_(L"module_custom_event: lua_pcall returned\n");
        processed = lua_toboolean(L, -2);
        if (lua_istable(L, -1)) {
            // registers
            registers_from_lua_table(L, -1, regs);
        }
        lua_pop(L,2);
        LeaveCriticalSection(&_cs);
    }
    return processed;
}

void module_set_teams(module_t *m, DWORD home, DWORD away) //, TEAM_INFO_STRUCT *home_team_info, TEAM_INFO_STRUCT *away_team_info)
{
    if (m->evt_set_teams != 0) {
        EnterCriticalSection(&_cs);
        lua_pushvalue(m->L, m->evt_set_teams);
        lua_xmove(m->L, L, 1);
        // push params
        lua_pushvalue(L, 1); // ctx
        lua_pushinteger(L, home);
        lua_pushinteger(L, away);
        //lua_pushlightuserdata(L, home_team_info);
        //lua_pushlightuserdata(L, away_team_info);
        if (lua_pcall(L, 3, 0, 0) != LUA_OK) {
            const char *err = luaL_checkstring(L, -1);
            logu_("[%d] lua ERROR from module_set_teams: %s\n", GetCurrentThreadId(), err);
            lua_pop(L, 1);
        }
        LeaveCriticalSection(&_cs);
    }
}

void module_set_home_team_for_kits(module_t *m, DWORD team_id, bool is_edit_mode)
{
    if (m->evt_set_home_team_for_kits != 0) {
        EnterCriticalSection(&_cs);
        lua_pushvalue(m->L, m->evt_set_home_team_for_kits);
        lua_xmove(m->L, L, 1);
        // push params
        lua_pushvalue(L, 1); // ctx
        lua_pushinteger(L, team_id);
        lua_pushinteger(L, is_edit_mode);

        if (lua_pcall(L, 3, 0, 0) != LUA_OK) {
            const char *err = luaL_checkstring(L, -1);
            logu_("[%d] lua ERROR from module_set_home_team_for_kits: %s\n", GetCurrentThreadId(), err);
            lua_pop(L, 1);
        }
        LeaveCriticalSection(&_cs);
    }
}

void module_set_away_team_for_kits(module_t *m, DWORD team_id, bool is_edit_mode)
{
    if (m->evt_set_away_team_for_kits != 0) {
        EnterCriticalSection(&_cs);
        lua_pushvalue(m->L, m->evt_set_away_team_for_kits);
        lua_xmove(m->L, L, 1);
        // push params
        lua_pushvalue(L, 1); // ctx
        lua_pushinteger(L, team_id);
        lua_pushinteger(L, is_edit_mode);

        if (lua_pcall(L, 3, 0, 0) != LUA_OK) {
            const char *err = luaL_checkstring(L, -1);
            logu_("[%d] lua ERROR from module_set_away_team_for_kits: %s\n", GetCurrentThreadId(), err);
            lua_pop(L, 1);
        }
        LeaveCriticalSection(&_cs);
    }
}

bool module_set_kits(module_t *m, MATCH_INFO_STRUCT *mi)
{
    BYTE *home_ki, *away_ki;
    bool result(false);
    if (m->evt_set_kits != 0) {
        EnterCriticalSection(&_cs);
        lua_pushvalue(m->L, m->evt_set_kits);
        lua_xmove(m->L, L, 1);
        // push params
        lua_pushvalue(L, 1); // ctx

        int home_kit_id = mi->home_player_kit_id;
        if (home_kit_id > 9) { home_kit_id = 9; }
        if (home_kit_id < 0) { home_kit_id = 0; }
        lua_newtable(L); // home team info
        home_ki = find_kit_info(get_team_id(mi, 0), suffix_map[home_kit_id]);
        get_kit_info_to_lua_table(L, -1, home_ki);

        int away_kit_id = mi->away_player_kit_id;
        if (away_kit_id > 9) { away_kit_id = 9; }
        if (away_kit_id < 0) { away_kit_id = 0; }
        lua_newtable(L); // away team info
        away_ki = find_kit_info(get_team_id(mi, 1), suffix_map[away_kit_id]);
        get_kit_info_to_lua_table(L, -1, away_ki);

        logu_("uniparam: %p\n", get_uniparam());

        if (lua_pcall(L, 3, 2, 0) != LUA_OK) {
            const char *err = luaL_checkstring(L, -1);
            logu_("[%d] lua ERROR from module_set_kits: %s\n", GetCurrentThreadId(), err);
            lua_pop(L, 1);
            LeaveCriticalSection(&_cs);
            return false;
        }
        if (lua_istable(L, -2)) {
            // home table
            BYTE *radar_color = (home_kit_id < 2) ? _mi->home.players[home_kit_id].color1 : _mi->home.extra_players[home_kit_id-2].color1;
            set_kit_info_from_lua_table(L, -2, home_ki, radar_color, NULL);
        }
        if (lua_istable(L, -1)) {
            // away table
            BYTE *radar_color = (away_kit_id < 2) ? _mi->away.players[away_kit_id].color1 : _mi->away.extra_players[away_kit_id-2].color1;
            set_kit_info_from_lua_table(L, -1, away_ki, radar_color, NULL);
        }
        lua_pop(L,2);
        LeaveCriticalSection(&_cs);
    }
    return result;
}

char *module_ball_name(module_t *m, char *name)
{
    char *res = NULL;
    if (m->evt_get_ball_name != 0) {
        EnterCriticalSection(&_cs);
        lua_pushvalue(m->L, m->evt_get_ball_name);
        lua_xmove(m->L, L, 1);
        // push params
        lua_pushvalue(L, 1); // ctx
        lua_pushstring(L, name);
        if (lua_pcall(L, 2, 1, 0) != LUA_OK) {
            const char *err = luaL_checkstring(L, -1);
            logu_("[%d] lua ERROR from module_ball_name: %s\n", GetCurrentThreadId(), err);
        }
        else if (lua_isstring(L, -1)) {
            const char *s = luaL_checkstring(L, -1);
            memset(_ball_name, 0, sizeof(_ball_name));
            strncpy(_ball_name, s, sizeof(_ball_name)-1);
            res = _ball_name;
        }
        lua_pop(L, 1);
        LeaveCriticalSection(&_cs);
    }
    return res;
}

char *module_stadium_name(module_t *m, char *name, BYTE stadium_id, SCHEDULE_ENTRY *se)
{
    char *res = NULL;
    if (m->evt_get_stadium_name != 0) {
        EnterCriticalSection(&_cs);
        lua_pushvalue(m->L, m->evt_get_stadium_name);
        lua_xmove(m->L, L, 1);
        // push params
        lua_pushvalue(L, 1); // ctx
        lua_pushstring(L, name);
        lua_pushinteger(L, stadium_id);
        if (se) {
            lua_newtable(L);
            lua_pushinteger(L, se->tournament_id);
            lua_setfield(L, -2, "tournament_id");
            lua_pushinteger(L, se->match_info);
            lua_setfield(L, -2, "match_info");
            lua_pushinteger(L, decode_team_id(se->home_team_encoded));
            lua_setfield(L, -2, "home_team");
            lua_pushinteger(L, decode_team_id(se->away_team_encoded));
            lua_setfield(L, -2, "away_team");
        }
        else {
            lua_pushnil(L);
        }
        if (lua_pcall(L, 4, 1, 0) != LUA_OK) {
            const char *err = luaL_checkstring(L, -1);
            logu_("[%d] lua ERROR from module_stadium_name: %s\n", GetCurrentThreadId(), err);
        }
        else if (lua_isstring(L, -1)) {
            const char *s = luaL_checkstring(L, -1);
            memset(_stadium_name, 0, sizeof(_stadium_name));
            strncpy(_stadium_name, s, sizeof(_stadium_name)-1);
            res = _stadium_name;
        }
        lua_pop(L, 1);
        LeaveCriticalSection(&_cs);
    }
    return res;
}

void module_show(module_t *m)
{
    if (m->evt_show != 0) {
        EnterCriticalSection(&_cs);
        lua_pushvalue(m->L, m->evt_show);
        lua_xmove(m->L, L, 1);
        // push params
        lua_pushvalue(L, 1); // ctx
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            const char *err = luaL_checkstring(L, -1);
            logu_("[%d] lua ERROR from module_show: %s\n", GetCurrentThreadId(), err);
            lua_pop(L, 1);
        }
        LeaveCriticalSection(&_cs);
    }
}

void module_hide(module_t *m)
{
    EnterCriticalSection(&_cs);
    if (m->evt_hide != 0) {
        lua_pushvalue(m->L, m->evt_hide);
        lua_xmove(m->L, L, 1);
        // push params
        lua_pushvalue(L, 1); // ctx
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            const char *err = luaL_checkstring(L, -1);
            logu_("[%d] lua ERROR from module_hide: %s\n", GetCurrentThreadId(), err);
            lua_pop(L, 1);
        }
    }
    _block_input = false;
    _hard_block = false;
    LeaveCriticalSection(&_cs);
}

void module_on_frame(module_t *m)
{
    if (m->evt_display_frame != 0) {
        EnterCriticalSection(&_cs);
        lua_pushvalue(m->L, m->evt_display_frame);
        lua_xmove(m->L, L, 1);
        // push params
        lua_pushvalue(L, 1); // ctx
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            const char *err = luaL_checkstring(L, -1);
            logu_("[%d] lua ERROR from module_display_frame: %s\n", GetCurrentThreadId(), err);
            lua_pop(L, 1);
        }
        LeaveCriticalSection(&_cs);
    }
}

void module_overlay_on(module_t *m, char **text, char **image_path, struct layout_t *opts)
{
    *text = NULL;
    *image_path = NULL;
    if (m->evt_overlay_on != 0) {
        PERF_TIMER(_overlay_stats);
        EnterCriticalSection(&_cs);
        // garbage collection
        lua_gc(L, _config->_lua_gc_opt, 0);
        lua_pushvalue(m->L, m->evt_overlay_on);
        lua_xmove(m->L, L, 1);
        // push params
        lua_pushvalue(L, 1); // ctx
        if (lua_pcall(L, 1, 3, 0) != LUA_OK) {
            const char *err = luaL_checkstring(L, -1);
            logu_("[%d] lua ERROR from module_overlay_on: %s\n", GetCurrentThreadId(), err);
            lua_pop(L, 1);
        }
        else {
            // check return values
            if (lua_isstring(L, -3)) {
                const char *s = luaL_checkstring(L, -3);
                _overlay_utf8_text[0] = '\0';
                strncat(_overlay_utf8_text, s, sizeof(_overlay_utf8_text)-1);
                *text = _overlay_utf8_text;
                // strip any trailing whitespace
                char *res = *text;
                if (s[0] != '\0') {
                    char *p = res + strlen(res) - 1;
                    while ((p >= res) && ((p[0] == '\r') || (p[0] == '\n') || (p[0] == ' '))) {
                        p[0] = '\0';
                        p--;
                    }
                }
            }
            if (lua_isstring(L, -2)) {
                const char *s = luaL_checkstring(L, -2);
                _overlay_utf8_image_path[0] = '\0';
                strncat(_overlay_utf8_image_path, s, sizeof(_overlay_utf8_image_path)-1);
                *image_path = _overlay_utf8_image_path;
            }
            if (lua_istable(L, -1)) {
                lua_getfield(L, -1, "image_width");
                if (lua_isnumber(L, -1)) {
                    double value = lua_tonumber(L, -1);
                    opts->image_width = value; // width in pixels
                    if (value < 1.0f) {
                        // treat as screen-width percentage
                        opts->image_width = DX11.Width * value;
                    }
                }
                lua_pop(L, 1);
                lua_getfield(L, -1, "image_height");
                if (lua_isnumber(L, -1)) {
                    double value = lua_tonumber(L, -1);
                    opts->image_height = value; // height in pixels
                    if (value < 1.0f) {
                        // treat as screen-height percentage
                        opts->image_height = DX11.Height * value;
                    }
                }
                lua_pop(L, 1);
                lua_getfield(L, -1, "image_aspect_ratio");
                if (lua_isnumber(L, -1)) {
                    double value = lua_tonumber(L, -1);
                    opts->image_aspect_ratio = value;
                    opts->has_image_ar = true;
                }
                lua_pop(L, 1);
                lua_getfield(L, -1, "image_hmargin");
                if (lua_isnumber(L, -1)) {
                    opts->image_hmargin = lua_tointeger(L, -1); // hmargin in pixels
                    opts->has_image_hmargin = true;
                }
                lua_pop(L, 1);
                lua_getfield(L, -1, "image_vmargin");
                if (lua_isnumber(L, -1)) {
                    opts->image_vmargin = lua_tointeger(L, -1); // vmargin in pixels
                    opts->has_image_vmargin = true;
                }
                lua_pop(L, 1);
            }
            lua_pop(L, 3);
        }
        LeaveCriticalSection(&_cs);
    }
}

void module_key_down(module_t *m, int vkey)
{
    if (m->evt_key_down != 0) {
        EnterCriticalSection(&_cs);
        lua_pushvalue(m->L, m->evt_key_down);
        lua_xmove(m->L, L, 1);
        // push params
        lua_pushvalue(L, 1); // ctx
        lua_pushinteger(L, vkey); // ctx
        if (lua_pcall(L, 2, 0, 0) != LUA_OK) {
            const char *err = luaL_checkstring(L, -1);
            logu_("[%d] lua ERROR from module_key_down: %s\n", GetCurrentThreadId(), err);
            lua_pop(L, 1);
        }
        LeaveCriticalSection(&_cs);
    }
}

void module_key_up(module_t *m, int vkey)
{
    if (m->evt_key_up != 0) {
        EnterCriticalSection(&_cs);
        lua_pushvalue(m->L, m->evt_key_up);
        lua_xmove(m->L, L, 1);
        // push params
        lua_pushvalue(L, 1); // ctx
        lua_pushinteger(L, vkey); // ctx
        if (lua_pcall(L, 2, 0, 0) != LUA_OK) {
            const char *err = luaL_checkstring(L, -1);
            logu_("[%d] lua ERROR from module_key_up: %s\n", GetCurrentThreadId(), err);
            lua_pop(L, 1);
        }
        LeaveCriticalSection(&_cs);
    }
}

void module_gamepad_input(module_t *m, struct xi_change_t *changes, size_t len)
{
    if (m->evt_gamepad_input != 0) {
        EnterCriticalSection(&_cs);
        lua_pushvalue(m->L, m->evt_gamepad_input);
        lua_xmove(m->L, L, 1);
        // push params
        lua_pushvalue(L, 1); // ctx
        lua_newtable(L); // table of changes
        for (int i=0; i<len; i++) {
            lua_pushstring(L, _xi_utf8_names[changes[i].what]);
            lua_pushinteger(L, changes[i].state);
            lua_settable(L, -3);
        }
        if (lua_pcall(L, 2, 0, 0) != LUA_OK) {
            const char *err = luaL_checkstring(L, -1);
            logu_("[%d] lua ERROR from module_gamepad_input: %s\n", GetCurrentThreadId(), err);
            lua_pop(L, 1);
        }
        LeaveCriticalSection(&_cs);
    }
    else if (m->evt_key_down != 0) {
        // check global gamepad xinput mapping
        for (int i=0; i<len; i++) {
            BYTE vkey;
            bool mapped = _gamepad_config->lookup(changes[i].what, changes[i].state, &vkey);
            if (!mapped) {
                continue;
            }
            DBG(256) logu_("for input event (%s,%d) found mapped vkey: 0x%x\n", _xi_utf8_names[changes[i].what], changes[i].state, vkey);
            EnterCriticalSection(&_cs);
            lua_pushvalue(m->L, m->evt_key_down);
            lua_xmove(m->L, L, 1);
            // push params
            lua_pushvalue(L, 1); // ctx
            lua_pushinteger(L, vkey); // ctx
            if (lua_pcall(L, 2, 0, 0) != LUA_OK) {
                const char *err = luaL_checkstring(L, -1);
                logu_("[%d] lua ERROR from module_gamepad_input (key-down): %s\n", GetCurrentThreadId(), err);
                lua_pop(L, 1);
            }
            LeaveCriticalSection(&_cs);
        }
    }
}

char *module_rewrite(module_t *m, const char *file_name)
{
    char *res(NULL);
    EnterCriticalSection(&_cs);
    lua_pushvalue(m->L, m->evt_lcpk_rewrite);
    lua_xmove(m->L, L, 1);
    // push params
    lua_pushvalue(L, 1); // ctx
    lua_pushstring(L, file_name);
    if (lua_pcall(L, 2, 1, 0) != LUA_OK) {
        const char *err = luaL_checkstring(L, -1);
        logu_("[%d] lua ERROR from module_rewrite: %s\n", GetCurrentThreadId(), err);
    }
    else if (lua_isstring(L, -1)) {
        const char *s = luaL_checkstring(L, -1);
        res = strdup(s);
    }
    lua_pop(L, 1);
    LeaveCriticalSection(&_cs);
    return res;
}

void module_read(module_t *m, const char *file_name, void *data, LONGLONG len, FILE_LOAD_INFO *fli)
{
    EnterCriticalSection(&_cs);
    lua_pushvalue(m->L, m->evt_lcpk_read);
    lua_xmove(m->L, L, 1);
    // push params
    lua_pushvalue(L, 1); // ctx
    lua_pushstring(L, file_name);
    lua_pushlightuserdata(L, data);
    lua_pushinteger(L, len);
    if (fli) {
        lua_pushinteger(L, fli->total_bytes_to_read);
        lua_pushinteger(L, fli->bytes_read_so_far);
    } else {
        lua_pushnil(L);
        lua_pushnil(L);
    }
    if (lua_pcall(L, 6, 0, 0) != LUA_OK) {
        const char *err = luaL_checkstring(L, -1);
        logu_("[%d] lua ERROR from module_read: %s\n", GetCurrentThreadId(), err);
        lua_pop(L, 1);
    }
    LeaveCriticalSection(&_cs);
}

void module_data_ready(module_t *m, const char *file_name, void *data, LONGLONG len, LONGLONG total_size, LONGLONG offset, const char *cpk_name)
{
    EnterCriticalSection(&_cs);
    lua_pushvalue(m->L, m->evt_lcpk_data_ready);
    lua_xmove(m->L, L, 1);
    // push params
    lua_pushvalue(L, 1); // ctx
    lua_pushstring(L, file_name);
    lua_pushlightuserdata(L, data);
    lua_pushinteger(L, len);
    lua_pushinteger(L, total_size);
    lua_pushinteger(L, offset);
    lua_pushstring(L, cpk_name);
    if (lua_pcall(L, 7, 0, 0) != LUA_OK) {
        const char *err = luaL_checkstring(L, -1);
        logu_("[%d] lua ERROR from module_data_ready: %s\n", GetCurrentThreadId(), err);
        lua_pop(L, 1);
    }
    LeaveCriticalSection(&_cs);
}

void module_make_key(module_t *m, const char *file_name, char *key, size_t key_maxsize)
{
    key[0] = '\0';
    size_t maxlen = key_maxsize-1;
    if (m->evt_lcpk_make_key != 0) {
        lock_t lock(&_cs);
        // garbage collection
        lua_gc(L, _config->_lua_gc_opt, 0);
        lua_pushvalue(m->L, m->evt_lcpk_make_key);
        lua_xmove(m->L, L, 1);
        // push params
        lua_pushvalue(L, 1); // ctx
        lua_pushstring(L, file_name);
        if (lua_pcall(L, 2, 1, 0) != LUA_OK) {
            const char *err = luaL_checkstring(L, -1);
            logu_("[%d] lua ERROR from module_make_key: %s\n", GetCurrentThreadId(), err);
            // fallback to filename
            strncat(key, file_name, maxlen);
        }
        else if (lua_isstring(L, -1)) {
            const char *s = luaL_checkstring(L, -1);
            strncat(key, s, maxlen);
        }
        else {
            // fallback to filename
            strncat(key, file_name, maxlen);
        }
        lua_pop(L, 1);
    }
    else {
        // default to filename
        strncat(key, file_name, maxlen);
    }
}

wchar_t *module_get_filepath(module_t *m, const char *file_name, char *key)
{
    wchar_t *res = NULL;
    if (m->evt_lcpk_get_filepath != 0) {
        lock_t lock(&_cs);
        lua_pushvalue(m->L, m->evt_lcpk_get_filepath);
        lua_xmove(m->L, L, 1);
        // push params
        lua_pushvalue(L, 1); // ctx
        lua_pushstring(L, file_name);
        lua_pushstring(L, (key[0]=='\0') ? NULL : key);
        if (lua_pcall(L, 3, 1, 0) != LUA_OK) {
            const char *err = luaL_checkstring(L, -1);
            logu_("[%d] lua ERROR from module_get_filepath: %s\n",
                GetCurrentThreadId(), err);
        }
        else if (lua_isstring(L, -1)) {
            const char *s = luaL_checkstring(L, -1);
            res = Utf8::utf8ToUnicode((void*)s);
            //wchar_t *ws = Utf8::utf8ToUnicode((void*)s);
            //res = new wstring(ws);
            //Utf8::free(ws);
        }
        lua_pop(L, 1);

        // verify that file exists
        if (res && !file_exists(res, NULL)) {
            delete res;
            res = NULL;
        }
    }
    return res;
}

bool do_rewrite(char *file_name)
{
    char key[512];
    char *res = NULL;

    /***
    if (_config->_rewrite_cache_ttl_sec) {
        if (_rewrite_cache->lookup(file_name, (void**)&res)) {
            // rewrite-cache: for performance
            if (res) {
                strcpy(file_name, res);
            }
            return res != NULL;
        }
    }
    ***/

    vector<module_t*>::iterator i;
    for (i = _modules.begin(); i != _modules.end(); i++) {
        module_t *m = *i;
        if (m->evt_lcpk_rewrite != 0) {
            res = module_rewrite(m, file_name);
            if (res) {
                //if (_config->_rewrite_cache_ttl_sec) _rewrite_cache->put(file_name, res);
                strcpy(file_name, res);
                return true;
            }
        }
    }

    //if (_config->_rewrite_cache_ttl_sec) _rewrite_cache->put(file_name, res);
    return false;
}

wchar_t* have_content(char *file_name)
{
    PERF_TIMER(_content_stats);
    char key[512];
    wchar_t *res = NULL;

    vector<module_t*>::iterator i;
    //logu_("have_content: %p --> %s\n", (DWORD)file_name, file_name);
    for (i = _modules.begin(); i != _modules.end(); i++) {
        module_t *m = *i;
        if (!m->evt_lcpk_make_key && !m->evt_lcpk_get_filepath) {
            // neither of callbacks is defined --> nothing to do
            continue;
        }

        module_make_key(m, file_name, key, sizeof(key));

        res = module_get_filepath(m, file_name, key);
        if (res) {
            return res;
        }
    }
    return NULL;
}

__declspec(dllexport) bool start_minimized()
{
    return _config && _config->_start_minimized;
}

inline char *get_tailname(char *filename)
{
    char *tail = filename + strlen(filename) + 1;
    if (*(DWORD*)tail == MAGIC) {
        return tail+5;
    }
    return filename;
}

bool have_cached(const char *file_name, wchar_t **res)
{
    if (_config->_key_cache_ttl_sec) {
        if (_small_key_cache->lookup(file_name, (void**)res)) {
            // first level cache (small)
            return true;
        }
        //if (_key_cache->lookup(file_name, (void**)res)) {
        //    // key-cache: for performance
        //    return true;
        //}
    }
    return false;
}

void cache_it(const char *file_name, wchar_t *res)
{
    if (_config->_key_cache_ttl_sec) {
        //_key_cache->put(file_name, res);
        _small_key_cache->put(file_name, res);
    }
}

void sider_get_size(char *filename, struct FILE_INFO *fi)
{
    char *fname = get_tailname(filename);
    if (fname[0]=='\0') {
        // no tail name: nothing to do
        return;
    }
    DBG(4) logu_("get_size:: tailname: %s\n", fname);

    wchar_t *fn(NULL);
    { PERF_TIMER(_fileops_stats); if (!have_cached(fname, &fn)) {
        if (_config->_lua_enabled && _rewrite_count > 0) do_rewrite(fname);
        fn = (_config->_lua_enabled) ? have_content(fname) : NULL;
        fn = (fn) ? fn : have_live_file(fname);
        cache_it(fname, fn);
    }}
    if (fn != NULL) {
        DBG(4) log_(L"get_size:: livecpk file found: %s\n", fn);
        HANDLE handle = CreateFileW(fn,  // file to open
                           GENERIC_READ,          // open for reading
                           FILE_SHARE_READ,       // share for reading
                           NULL,                  // default security
                           OPEN_EXISTING,         // existing file only
                           FILE_ATTRIBUTE_NORMAL, // normal file
                           NULL);                 // no attr. template

        if (handle != INVALID_HANDLE_VALUE)
        {
            DWORD sz = GetFileSize(handle, NULL);
            DBG(4) log_(L"get_size:: livecpk file size: %x\n", sz);
            CloseHandle(handle);
            fi->size = sz;
            fi->size_uncompressed = sz;
            //fi->offset_in_cpk = 0;

            // restore the tail name
            strcpy(filename, fname);
        }
    }
}

void prep_stuff()
{
/*
    log_(L"Loading D3DCOMPILER_DLL = {%s} ...\n", D3DCOMPILER_DLL);
    if (LoadLibrary(D3DCOMPILER_DLL)) {
        log_(L"Loaded D3DCOMPILER_DLL = %s\n", D3DCOMPILER_DLL);
    }
    else {
        log_(L"Failed to load D3DCOMPILER_DLL (%s). Error: %d\n", D3DCOMPILER_DLL, GetLastError());
    }
*/
    hr = FW1CreateFactory(FW1_VERSION, &g_pFW1Factory);
    if (FAILED(hr)) {
        logu_("FW1CreateFactory failed with: %p\n", hr);
        return;
    }
    logu_("FW1CreateFactory: %p\n", g_pFW1Factory);
	hr = g_pFW1Factory->CreateFontWrapper(DX11.Device, L"Arial", &g_pFontWrapper);
    if (FAILED(hr)) {
        logu_("CreateFontWrapper failed with: %p\n", hr);
        return;
    }
    logu_("FW1FontWrapper: %p\n", g_pFontWrapper);

    DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
    ID3D10Blob* pBlobError = NULL;
/*
    ID3D10Blob* pBlobVS = NULL;
    hr = D3DCompile(g_strVS, lstrlenA(g_strVS) + 1, "VS", NULL, NULL, "VS",
        "vs_4_0", dwShaderFlags, 0, &pBlobVS, &pBlobError);
    if (FAILED(hr))
    {
        if (pBlobError != NULL)
        {
            logu_((char*)pBlobError->GetBufferPointer());
            pBlobError->Release();
        }
        logu_("D3DCompile failed\n");
        return;
    }
    hr = DX11.Device->CreateVertexShader(pBlobVS->GetBufferPointer(), pBlobVS->GetBufferSize(),
        NULL, &g_pVertexShader);

    ID3D10Blob* pBlobTexVS = NULL;
    ID3D10Blob* pBlobTexError = NULL;
    hr = D3DCompile(g_strTexVS, lstrlenA(g_strTexVS) + 1, "VStex", NULL, NULL, "VStex",
        "vs_4_0", dwShaderFlags, 0, &pBlobTexVS, &pBlobTexError);
    if (FAILED(hr))
    {
        if (pBlobError != NULL)
        {
            logu_((char*)pBlobError->GetBufferPointer());
            pBlobTexError->Release();
        }
        logu_("D3DCompile failed\n");
        return;
    }
    hr = DX11.Device->CreateVertexShader(pBlobTexVS->GetBufferPointer(), pBlobTexVS->GetBufferSize(),
        NULL, &g_pTexVertexShader);

*/
#include "vshader.h"
    logu_("creating vertex shader from array of %d bytes\n", sizeof(g_siderVS));
    hr = DX11.Device->CreateVertexShader(g_siderVS, sizeof(g_siderVS), NULL, &g_pVertexShader);
    if (FAILED(hr)) {
        logu_("DX11.Device->CreateVertexShader failed\n");
        return;
    }

#include "vtexshader.h"
    logu_("creating vertex shader from array of %d bytes\n", sizeof(g_siderTexVS));
    hr = DX11.Device->CreateVertexShader(g_siderTexVS, sizeof(g_siderTexVS), NULL, &g_pTexVertexShader);
    if (FAILED(hr)) {
        logu_("DX11.Device->CreateVertexShader failed\n");
        return;
    }

/*
    // Compile and create the pixel shader
    ID3D10Blob* pBlobPS = NULL;
    char pixel_shader[512];
    memset(pixel_shader, 0, sizeof(pixel_shader));
    float r = float(_config->_overlay_background_color & 0x00ff)/255.0;
    float g = float((_config->_overlay_background_color & 0x00ff00) >> 8)/255.0;
    float b = float((_config->_overlay_background_color & 0x00ff0000) >> 16)/255.0;
    float a = float((_config->_overlay_background_color & 0x00ff000000) >> 24)/255.0;
    sprintf(pixel_shader, g_strPS, r, g, b, a);
    hr = D3DCompile(pixel_shader, lstrlenA(pixel_shader) + 1, "PS", NULL, NULL, "PS",
        "ps_4_0", dwShaderFlags, 0, &pBlobPS, &pBlobError);
    if (FAILED(hr))
    {
        if (pBlobError != NULL)
        {
            logu_((char*)pBlobError->GetBufferPointer());
            pBlobError->Release();
        }
        logu_("D3DCompile failed\n");
        return;
    }
    hr = DX11.Device->CreatePixelShader(pBlobPS->GetBufferPointer(), pBlobPS->GetBufferSize(),
        NULL, &g_pPixelShader);
    if (FAILED(hr)) {
        logu_("DX11.Device->CreatePixelShader failed\n");
        return;
    }
    pBlobPS->Release();

    // Compile and create another pixel shader
    pBlobPS = NULL;
    memset(pixel_shader, 0, sizeof(pixel_shader));
    sprintf(pixel_shader, g_strTexPS, _config->_overlay_image_alpha_max);
    hr = D3DCompile(pixel_shader, strlen(pixel_shader) + 1, "PStex", NULL, NULL, "PStex",
        "ps_4_0", dwShaderFlags, 0, &pBlobPS, &pBlobError);
    if (FAILED(hr))
    {
        if (pBlobError != NULL)
        {
            logu_((char*)pBlobError->GetBufferPointer());
            pBlobError->Release();
        }
        logu_("D3DCompile failed\n");
        return;
    }
    hr = DX11.Device->CreatePixelShader(pBlobPS->GetBufferPointer(), pBlobPS->GetBufferSize(),
        NULL, &g_pTexPixelShader);
    if (FAILED(hr)) {
        logu_("DX11.Device->CreatePixelShader failed\n");
        return;
    }
    pBlobPS->Release();
*/
#include "pshader.h"
    logu_("creating pixel shader from array of %d bytes\n", sizeof(g_siderPS));
    hr = DX11.Device->CreatePixelShader(g_siderPS, sizeof(g_siderPS), NULL, &g_pPixelShader);
    if (FAILED(hr)) {
        logu_("DX11.Device->CreatePixelShader failed\n");
        return;
    }

#include "ptexshader.h"
    logu_("creating pixel shader from array of %d bytes\n", sizeof(g_siderTexPS));
    hr = DX11.Device->CreatePixelShader(g_siderTexPS, sizeof(g_siderTexPS), NULL, &g_pTexPixelShader);
    if (FAILED(hr)) {
        logu_("DX11.Device->CreatePixelShader failed\n");
        return;
    }

    // Create the input layout
    D3D11_INPUT_ELEMENT_DESC elements[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(float)*4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = _countof(elements);
    //hr = DX11.Device->CreateInputLayout(elements, numElements, pBlobVS->GetBufferPointer(),
    //    pBlobVS->GetBufferSize(), &g_pInputLayout);
    hr = DX11.Device->CreateInputLayout(elements, numElements, g_siderVS,
        sizeof(g_siderVS), &g_pInputLayout);
    if (FAILED(hr)) {
        logu_("DX11.Device->CreateInputLayout failed\n");
        return;
    }

    // Create the input layout for texture
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(float)*4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    numElements = _countof(layout);
    //hr = DX11.Device->CreateInputLayout(layout, numElements, pBlobTexVS->GetBufferPointer(),
    //    pBlobTexVS->GetBufferSize(), &g_pTexInputLayout);
    hr = DX11.Device->CreateInputLayout(layout, numElements, g_siderTexVS,
        sizeof(g_siderTexVS), &g_pTexInputLayout);
    if (FAILED(hr)) {
        logu_("DX11.Device->CreateInputLayout failed\n");
        return;
    }

    // define and set the constant buffers
    // constant buffer
    D3D11_BUFFER_DESC bd = { 0 };
    bd.ByteWidth = 16;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = &g_constants;

    if (FAILED(DX11.Device->CreateBuffer(&bd, &initData, &g_pConstantBuffer))) {
        logu_("DX11.Device->CreateBuffer failed for constant buffer\n");
        return;
    }

    //pBlobVS->Release();
    //pBlobTexVS->Release();

    // Create the state objects
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = DX11.Device->CreateSamplerState( &sampDesc, &g_pSamplerLinear );
    if (FAILED(hr)) {
        logu_("DX11.Device->CreateSamplerState failed\n");
        return;
    }

    D3D11_BLEND_DESC BlendState;
    ZeroMemory(&BlendState, sizeof(D3D11_BLEND_DESC));

    BlendState.RenderTarget[0].BlendEnable = TRUE;
    BlendState.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    BlendState.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    BlendState.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    BlendState.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    BlendState.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    BlendState.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    BlendState.RenderTarget[0].RenderTargetWriteMask = 0x0f;
    DX11.Device->CreateBlendState(&BlendState, &g_pBlendState);

    // default to automatic font size
    _font_size = DX11.Height/45.0;
    if (_config->_overlay_font_size > 0) {
        _font_size = (float)_config->_overlay_font_size;
    }

    logu_("prep done successfully!\n");
}

int prep_ui(float font_size, float right_margin)
{
    swprintf(_overlay_text, L"%s | %s | %s", _overlay_header.c_str(), (*_curr_overlay_m)->filename->c_str(), _current_overlay_text);
    UINT flags = 0; //FW1_RESTORESTATE;
    //if (_config->_overlay_location == 1) {
    //    flags |= FW1_BOTTOM;
    //}

    FW1_RECTF rectIn;
    rectIn.Left = 5.0f;
    rectIn.Top = 0.0f;
    rectIn.Right = DX11.Width - right_margin;
    rectIn.Bottom = DX11.Height;
	FW1_RECTF rect = g_pFontWrapper->MeasureString(_overlay_text, _config->_overlay_font.c_str(), font_size, &rectIn, flags);
    //logu_("rect: %0.2f,%0.2f,%0.2f,%0.2f\n", rect.Left,rect.Top,rect.Right,rect.Bottom);
    float height = rect.Bottom;
    if (height < 0) { height = DX11.Height + height; }
    float rel_height = (height + rect.Top) / DX11.Height + 0.005;

    if (_overlay_image.have && _overlay_image.height > 0) {
        float rel_image_height = ((float)_overlay_image.height + _overlay_image.vmargin*2) / DX11.Height;
        rel_height = max(rel_height, rel_image_height);
    }
    //logu_("rel_height: %0.2f\n", rel_height);
    int pixel_height = rel_height * DX11.Height;

    // overlay
    {
        D3D11_BUFFER_DESC bd;
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(g_vertices);
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = 0;
        bd.MiscFlags = 0;
        bd.StructureByteStride = 0;
        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = g_vertices;
        hr = DX11.Device->CreateBuffer(&bd, &initData, &g_pVertexBuffer);
        if (FAILED(hr)) {
            logu_("DX11.Device->CreateBuffer failed (for overlay)\n");
            return 0;
        }
    }

    // image
    if (_overlay_image.have) {
        D3D11_BUFFER_DESC bd;
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(g_texVertices);
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = 0;
        bd.MiscFlags = 0;
        bd.StructureByteStride = 0;
        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = g_texVertices;
        hr = DX11.Device->CreateBuffer(&bd, &initData, &g_pTexVertexBuffer);
        if (FAILED(hr)) {
            logu_("DX11.Device->CreateBuffer failed (for image)\n");
            return 0;
        }
    }

    return pixel_height;
}

void draw_text(float font_size, float right_margin)
{
    UINT flags = FW1_RESTORESTATE;
    //FLOAT y = DX11.Height*0.0f;
    if (_config->_overlay_location == 1) {
        //flags |= FW1_BOTTOM;
        //y = DX11.Height*1.0f;
    }

    FW1_RECTF rectIn;
    rectIn.Left = 5.0f;
    rectIn.Top = 0.0f;
    rectIn.Right = DX11.Width - right_margin;
    rectIn.Bottom = DX11.Height;

	g_pFontWrapper->DrawString(
		DX11.Context,
        _overlay_text,
        _config->_overlay_font.c_str(),
		font_size,// Font size
		//DX11.Width*0.01f,// X position
		//y,// Y position
        &rectIn,
        _config->_overlay_text_color, //0xd080ff80 - Text color, 0xAaBbGgRr
        NULL, NULL,
		flags //0// Flags (for example FW1_RESTORESTATE to keep context states unchanged)
	);
	//pFontWrapper->Release();
	//pFW1Factory->Release();
}

void draw_ui(float top, float bottom, float right_margin)
{
    // Create the render target view
    ID3D11Texture2D* pRenderTargetTexture;
    hr = DX11.SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pRenderTargetTexture);
    if (FAILED(hr)) {
        logu_("DX11.SwapChain->GetBuffer failed\n");
        return;
    }

    hr = DX11.Device->CreateRenderTargetView(pRenderTargetTexture, NULL, &g_pRenderTargetView);
    if (FAILED(hr)) {
        logu_("DX11.Device->CreateRenderTargetView failed\n");
        pRenderTargetTexture->Release();
        return;
    }
    pRenderTargetTexture->Release();

    RECT rc;
    GetClientRect(DX11.Window, &rc);

    // draw overlay background
    {
        DX11.Context->IASetInputLayout(g_pInputLayout);

        UINT stride = sizeof(SimpleVertex);
        UINT offset = 0;
        DX11.Context->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
        DX11.Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        DX11.Context->VSSetShader(g_pVertexShader, NULL, 0);
        DX11.Context->PSSetShader(g_pPixelShader, NULL, 0);

        D3D11_VIEWPORT vp;
        vp.Width = (FLOAT)(rc.right - rc.left);
        vp.Height = (FLOAT)(bottom - top);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = top;

        DX11.Context->RSSetViewports(1, &vp);
        DX11.Context->OMSetRenderTargets(1, &g_pRenderTargetView, NULL);
        DX11.Context->OMSetBlendState(g_pBlendState, NULL, 0xffffffff);
        DX11.Context->Draw(6, 0); //6 vertices start at 0
    }

    // draw texture
    if (_overlay_image.have) {
        DX11.Context->IASetInputLayout(g_pTexInputLayout);

        UINT stride = sizeof(TexturedVertex);
        UINT offset = 0;
        DX11.Context->IASetVertexBuffers(0, 1, &g_pTexVertexBuffer, &stride, &offset);
        DX11.Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        DX11.Context->VSSetShader(g_pTexVertexShader, NULL, 0);
        DX11.Context->PSSetShader(g_pTexPixelShader, NULL, 0);
        DX11.Context->PSSetShaderResources( 0, 1, &g_textureView );
        DX11.Context->PSSetSamplers( 0, 1, &g_pSamplerLinear );
        DX11.Context->PSSetConstantBuffers( 0, 1, &g_pConstantBuffer );

        D3D11_VIEWPORT vp;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = rc.right - rc.left - _overlay_image.width - _overlay_image.hmargin;
        vp.TopLeftY = top + _overlay_image.vmargin;
        vp.Width = _overlay_image.width;
        vp.Height = _overlay_image.height;
        DX11.Context->RSSetViewports(1, &vp);
        DX11.Context->OMSetRenderTargets(1, &g_pRenderTargetView, NULL);

        //float bf [4] = {1.0f, 1.0f, 1.0f, 1.0f};
        DX11.Context->OMSetBlendState(g_pBlendState, NULL, 0xffffffff);
        DX11.Context->Draw(6, 0); //6 vertices start at 0
    }

    // text
    {
        D3D11_VIEWPORT vp;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = top;
        vp.Width = rc.right - rc.left;
        vp.Height = bottom - top;
        DX11.Context->RSSetViewports(1, &vp);
        draw_text(_font_size, right_margin);
    }

    //cleanup
    g_pRenderTargetView->Release();
}

void sider_dispatch_show_hide_events(bool on)
{
    if (_curr_overlay_m != _modules.end()) {
        if (on) {
            module_show(*_curr_overlay_m);
        }
        else {
            module_hide(*_curr_overlay_m);
        }
    }
}

void sider_switch_overlay_to_prev_module()
{
    if (_curr_overlay_m != _modules.end()) {
        module_hide(*_curr_overlay_m);
        // previous module
        vector<module_t*>::iterator j = _curr_overlay_m;
        do {
            if (j == _modules.begin()) {
                j = _modules.end();
            }
            j--;
            module_t *m = *j;
            if (m->evt_overlay_on) {
                log_(L"now active module on overlay: %s\n", m->filename->c_str());
                _curr_overlay_m = j;
                module_show(*_curr_overlay_m);
                break;
            }
        }
        while (j != _curr_overlay_m);
        _overlay_image.to_clear = true;
    }
}

void sider_switch_overlay_to_next_module()
{
    if (_curr_overlay_m != _modules.end()) {
        module_hide(*_curr_overlay_m);
        // next module
        vector<module_t*>::iterator j = _curr_overlay_m;
        do {
            j++;
            if (j == _modules.end()) {
                j = _modules.begin();
            }
            module_t *m = *j;
            if (m->evt_overlay_on) {
                log_(L"now active module on overlay: %s\n", m->filename->c_str());
                _curr_overlay_m = j;
                module_show(*_curr_overlay_m);
                break;
            }
        }
        while (j != _curr_overlay_m);
        _overlay_image.to_clear = true;
    }
}

void clear_overlay_texture() {
    SAFE_RELEASE(g_texture);
    SAFE_RELEASE(g_textureView);
    if (_overlay_image.filepath) { free(_overlay_image.filepath); }
    memset(&_overlay_image, 0, sizeof(overlay_image_t));
}

int get_stick_state(int state, int middle) {
    float val = (state - middle)/32767.0f;
    if (val < -_gamepad_config->_stick_sensitivity) { return -1; }
    else if (val > _gamepad_config->_stick_sensitivity) { return 1; }
    return 0;
}

DWORD direct_input_poll(void *param) {
    HRESULT hr;
    bool has_xinput(false);
    bool has_dinput(false);
    int xi_fail_count = 0;
    int xi_skip_count = 0;
    int di_fail_count = 0;
    int di_skip_count = 0;
    DWORD last_good_xi = 0;

    int skips = 50;

    while (_controller_poll) {
        bool input_processing_done = false;

        // XInput

        if (_gamepad_config->_xinput_enabled && xi_skip_count == 0) {
            // read XInput controller state
            DWORD dwResult;
            for (DWORD k=0; k<4; k++ ) {
                // start with the last one that was successfully queried
                int j = (last_good_xi + k) % 4;

                DBG(512) log_(L"query XInput controller %d\n", j);

                XINPUT_STATE xstate;
                ZeroMemory( &xstate, sizeof(XINPUT_STATE) );
                // Simply get the state of the controller from XInput.
                dwResult = XInputGetState( j, &xstate );
                if( dwResult == ERROR_SUCCESS ) {
                    // Controller is connected
                    xi_state_t state;
                    state.buttons[0] = (xstate.Gamepad.wButtons & 0x1000) ? 1 : 0;
                    state.buttons[1] = (xstate.Gamepad.wButtons & 0x2000) ? 1 : 0;
                    state.buttons[2] = (xstate.Gamepad.wButtons & 0x4000) ? 1 : 0;
                    state.buttons[3] = (xstate.Gamepad.wButtons & 0x8000) ? 1 : 0;
                    state.buttons[4] = (xstate.Gamepad.wButtons & 0x0100) ? 1 : 0;
                    state.buttons[5] = (xstate.Gamepad.wButtons & 0x0200) ? 1 : 0;
                    state.buttons[6] = (xstate.Gamepad.wButtons & 0x0010) ? 1 : 0;
                    state.buttons[7] = (xstate.Gamepad.wButtons & 0x0020) ? 1 : 0;
                    state.buttons[8] = (xstate.Gamepad.wButtons & 0x0040) ? 1 : 0;
                    state.buttons[9] = (xstate.Gamepad.wButtons & 0x0080) ? 1 : 0;
                    state.left_trigger = (xstate.Gamepad.bLeftTrigger > 100) ? 1 : 0;
                    state.right_trigger = (xstate.Gamepad.bRightTrigger > 100) ? 1 : 0;
                    state.left_stick_x = get_stick_state(xstate.Gamepad.sThumbLX, 128);
                    state.left_stick_y = get_stick_state(xstate.Gamepad.sThumbLY, 128);
                    state.right_stick_x = get_stick_state(xstate.Gamepad.sThumbRX, 128);
                    state.right_stick_y = get_stick_state(xstate.Gamepad.sThumbRY, 128);
                    state.dpad = xstate.Gamepad.wButtons & 0x000f;

                    if (memcmp(&state, &_prev_xi_state, sizeof(xi_state_t))!=0) {
                        _xi_changes_len = 0;
                        for (int i=0; i<10; i++) {
                            if (_prev_xi_state.buttons[i] != state.buttons[i]) {
                                _xi_changes[_xi_changes_len].state = state.buttons[i];
                                _xi_changes[_xi_changes_len].what = i;
                                _xi_changes_len++;
                            }
                        }
                        if (_prev_xi_state.left_trigger != state.left_trigger) {
                            _xi_changes[_xi_changes_len].state = state.left_trigger;
                            _xi_changes[_xi_changes_len].what = GAMEPAD::LT;
                            _xi_changes_len++;
                        }
                        if (_prev_xi_state.right_trigger != state.right_trigger) {
                            _xi_changes[_xi_changes_len].state = state.right_trigger;
                            _xi_changes[_xi_changes_len].what = GAMEPAD::RT;
                            _xi_changes_len++;
                        }
                        if (_prev_xi_state.left_stick_x != state.left_stick_x) {
                            _xi_changes[_xi_changes_len].state = state.left_stick_x;
                            _xi_changes[_xi_changes_len].what = GAMEPAD::LSx;
                            _xi_changes_len++;
                        }
                        if (_prev_xi_state.left_stick_y != state.left_stick_y) {
                            _xi_changes[_xi_changes_len].state = state.left_stick_y;
                            _xi_changes[_xi_changes_len].what = GAMEPAD::LSy;
                            _xi_changes_len++;
                        }
                        if (_prev_xi_state.right_stick_x != state.right_stick_x) {
                            _xi_changes[_xi_changes_len].state = state.right_stick_x;
                            _xi_changes[_xi_changes_len].what = GAMEPAD::RSx;
                            _xi_changes_len++;
                        }
                        if (_prev_xi_state.right_stick_y != state.right_stick_y) {
                            _xi_changes[_xi_changes_len].state = state.right_stick_y;
                            _xi_changes[_xi_changes_len].what = GAMEPAD::RSy;
                            _xi_changes_len++;
                        }
                        if (_prev_xi_state.dpad != state.dpad) {
                            _xi_changes[_xi_changes_len].state = state.dpad;
                            _xi_changes[_xi_changes_len].what = GAMEPAD::DPAD;
                            _xi_changes_len++;
                        }

                        // test for events
                        bool handled(false);

                        // overlay toggle
                        BYTE was_b1 = *(BYTE*)((BYTE*)&_prev_xi_state + _gamepad_config->_overlay_toggle_1);
                        BYTE was_b2 = *(BYTE*)((BYTE*)&_prev_xi_state + _gamepad_config->_overlay_toggle_2);
                        BYTE b1 = *(BYTE*)((BYTE*)&state + _gamepad_config->_overlay_toggle_1);
                        BYTE b2 = *(BYTE*)((BYTE*)&state + _gamepad_config->_overlay_toggle_2);
                        if (b1==1 && b2==1 && (was_b1!=b1 || was_b2!=b2)) {
                            _overlay_on = !_overlay_on;
                            play_overlay_toggle_sound();
                            sider_dispatch_show_hide_events(_overlay_on);
                            handled = true;
                            _toggle_sequence_on = true;
                            DBG(64) logu_("overlay: %s\n", (_overlay_on)?"ON":"OFF");
                        }
                        else if (!_toggle_sequence_on) {
                            // module switch
                            BYTE was_b = *(BYTE*)((BYTE*)&_prev_xi_state + _gamepad_config->_overlay_next_module);
                            BYTE b = *(BYTE*)((BYTE*)&state + _gamepad_config->_overlay_next_module);
                            if (b==0 && was_b!=b) {
                                if (_overlay_on) {
                                    sider_switch_overlay_to_next_module();
                                    handled = true;
                                }
                            }
                            else {
                                was_b = *(BYTE*)((BYTE*)&_prev_xi_state + _gamepad_config->_overlay_prev_module);
                                b = *(BYTE*)((BYTE*)&state + _gamepad_config->_overlay_prev_module);
                                if (b==0 && was_b!=b) {
                                    if (_overlay_on) {
                                        sider_switch_overlay_to_prev_module();
                                        handled = true;
                                    }
                                }
                            }
                        }

                        if (_toggle_sequence_on && b1==0 && b2==0) {
                            _toggle_sequence_on = false;
                        }

                        // check states
                        if (_overlay_on && !handled && _curr_overlay_m != _modules.end()) {
                            if (_xi_changes_len>0) {
                                DBG(256) {
                                    logu_("XInput:: number of input changes: %d\n", _xi_changes_len);
                                    for (int i=0; i<_xi_changes_len; i++) {
                                        logu_("XInput:: change: what=%s, state=%d\n", _xi_utf8_names[_xi_changes[i].what], _xi_changes[i].state);
                                    }
                                }
                                // lua callback: generate input-change event
                                module_t *m = *_curr_overlay_m;
                                module_gamepad_input(m, _xi_changes, _xi_changes_len);
                            }
                        }
                    }
                    memcpy(&_prev_xi_state, &state, sizeof(xi_state_t));

                    DBG(256) {
                        logu_("----------------------------------------\n");
                        logu_("state.Gamepad.wButtons = 0x%04x\n", xstate.Gamepad.wButtons);
                        logu_("state.Gamepad.LeftTrigger = 0x%02x\n", xstate.Gamepad.bLeftTrigger);
                        logu_("state.Gamepad.RightTrigger = 0x%02x\n", xstate.Gamepad.bRightTrigger);
                        logu_("state.Gamepad.sThumbLX = %d\n", xstate.Gamepad.sThumbLX);
                        logu_("state.Gamepad.sThumbLY = %d\n", xstate.Gamepad.sThumbLY);
                        logu_("state.Gamepad.sThumbRX = %d\n", xstate.Gamepad.sThumbRX);
                        logu_("state.Gamepad.sThumbRY = %d\n", xstate.Gamepad.sThumbRY);
                    }

                    // successfully read controller. We're done
                    input_processing_done = true;
                    xi_fail_count = 0;
                    last_good_xi = k;

                    // only poll one XInput controller: do not query others
                    // that may be connected as well
                    break;
                }
                else {
                    // Controller is not connected
                    //logu_("controller %d is not connected\n", i);
                }
            }

            if (!input_processing_done) {
                xi_fail_count = xi_fail_count + 1;
            }
        }

        if (xi_skip_count > 0) {
            xi_skip_count--;
        }
        if (xi_fail_count >= 5) {
            xi_fail_count--;
            xi_skip_count = skips;
        }

        set_controller_poll_delay();

        if (input_processing_done) {
            DBG(512) log_(L"Sleep for %d msec\n", _controller_poll_delay);
            Sleep(_controller_poll_delay);
            continue;
        }

        // DirectInput

        if (_has_controller && _gamepad_config->_dinput_enabled && di_skip_count == 0) {
            DBG(512) log_(L"query DirectInput controller\n");

            hr = g_IDirectInputDevice8->Acquire();
            if (SUCCEEDED(hr)) {
                memcpy(_prev_controller_buttons, _controller_buttons, sizeof(_controller_buttons));
                hr = g_IDirectInputDevice8->GetDeviceState(sizeof(_controller_buttons), _controller_buttons);
                if (SUCCEEDED(hr)) {
                    input_processing_done = true;
                    di_fail_count = 0;

                    // log changes
                    DBG(64) {
                        if (memcmp(_prev_controller_buttons, _controller_buttons, sizeof(_controller_buttons))!=0) {
                            logu_("was: ");
                            vector<DIDEVICEOBJECTINSTANCE>::iterator it;
                            for (it = _di_objects.begin(); it != _di_objects.end(); it++) {
                                if (it->dwType & DIDFT_AXIS) {
                                    log_(L"|%s (0x%x): %d\n", it->tszName, it->dwType, *(DWORD*)(_prev_controller_buttons + it->dwOfs));
                                }
                                else if (it->dwType & DIDFT_POV) {
                                    log_(L"|%s (0x%x): %d\n", it->tszName, it->dwType, *(DWORD*)(_prev_controller_buttons + it->dwOfs));
                                }
                                else {
                                    log_(L"|%s (0x%x): %d\n", it->tszName, it->dwType, _prev_controller_buttons[it->dwOfs]);
                                }
                            }
                            logu_("\n");
                            logu_("now: ");
                            for (it = _di_objects.begin(); it != _di_objects.end(); it++) {
                                if (it->dwType & DIDFT_AXIS) {
                                    log_(L"|%s (0x%x): %d\n", it->tszName, it->dwType, *(DWORD*)(_controller_buttons + it->dwOfs));
                                }
                                else if (it->dwType & DIDFT_POV) {
                                    log_(L"|%s (0x%x): %d\n", it->tszName, it->dwType, *(DWORD*)(_controller_buttons + it->dwOfs));
                                }
                                else {
                                    log_(L"|%s (0x%x): %d\n", it->tszName, it->dwType, _controller_buttons[it->dwOfs]);
                                }
                            }
                            logu_("\n");
                        }
                    }

                    // test for events
                    bool handled(false);

                    // overlay toggle
                    BYTE was_b1 = *(BYTE*)(_prev_controller_buttons + _di_overlay_toggle1.dwOfs);
                    BYTE was_b2 = *(BYTE*)(_prev_controller_buttons + _di_overlay_toggle2.dwOfs);
                    BYTE b1 = *(BYTE*)(_controller_buttons + _di_overlay_toggle1.dwOfs);
                    BYTE b2 = *(BYTE*)(_controller_buttons + _di_overlay_toggle2.dwOfs);
                    if (b1!=0 && b2!=0 && (was_b1!=b1 || was_b2!=b2)) {
                        _overlay_on = !_overlay_on;
                        play_overlay_toggle_sound();
                        sider_dispatch_show_hide_events(_overlay_on);
                        handled = true;
                        _toggle_sequence_on = true;
                        DBG(64) logu_("overlay: %s\n", (_overlay_on)?"ON":"OFF");
                    }
                    else if (!_toggle_sequence_on) {
                        // module switch
                        BYTE was_b = *(BYTE*)(_prev_controller_buttons + _di_module_switch.dwOfs);
                        BYTE b = *(BYTE*)(_controller_buttons + _di_module_switch.dwOfs);
                        if (b==0 && was_b!=b) {
                            if (_overlay_on) {
                                sider_switch_overlay_to_next_module();
                                handled = true;
                            }
                        }
                        else {
                            was_b = *(BYTE*)(_prev_controller_buttons + _di_module_switch_prev.dwOfs);
                            b = *(BYTE*)(_controller_buttons + _di_module_switch_prev.dwOfs);
                            if (b==0 && was_b!=b) {
                                if (_overlay_on) {
                                    sider_switch_overlay_to_prev_module();
                                    handled = true;
                                }
                            }
                        }
                    }

                    if (_toggle_sequence_on && b1==0 && b2==0) {
                        _toggle_sequence_on = false;
                    }

                    // check states
                    if (_overlay_on && !handled && _curr_overlay_m != _modules.end()) {
                        _xi_changes_len = 0;
                        vector<DIDEVICEOBJECTINSTANCE>::iterator it;
                        for (it = _di_objects.begin(); it != _di_objects.end(); it++) {
                            int what;
                            if (!_gamepad_config->lookup_di(it->dwType, &what)) {
                                continue;
                            }

                            if (it->dwType & DIDFT_AXIS) {
                                // sticks
                                int was = get_stick_state(*(int*)(_prev_controller_buttons + it->dwOfs), 32767);
                                int now = get_stick_state(*(int*)(_controller_buttons + it->dwOfs), 32767);
                                if (was != now) {
                                    if (what == GAMEPAD::LSy || what == GAMEPAD::RSy) {
                                        now = -now; // reverse vertical direction: for consistency with Xinput
                                    }
                                    _xi_changes[_xi_changes_len].state = now;
                                    _xi_changes[_xi_changes_len].what = what;
                                    _xi_changes_len++;
                                }
                            }
                            else if (it->dwType & DIDFT_POV) {
                                // d-pad
                                int was = *(int*)(_prev_controller_buttons + it->dwOfs);
                                int now = *(int*)(_controller_buttons + it->dwOfs);
                                if (was != now) {  // degree-like: 0/4500/9000/13500/27000/31500
                                    int state = (now == -1) ? 0 : _fast_pov_map[now / 100];
                                    _xi_changes[_xi_changes_len].state = state;
                                    _xi_changes[_xi_changes_len].what = what;
                                    _xi_changes_len++;
                                }
                            }
                            else {
                                // buttons
                                BYTE was = _prev_controller_buttons[it->dwOfs];
                                BYTE now = _controller_buttons[it->dwOfs];
                                if (was != now) { // down/up: 128/0
                                    int state = now / 128;
                                    _xi_changes[_xi_changes_len].state = state;
                                    _xi_changes[_xi_changes_len].what = what;
                                    _xi_changes_len++;
                                }
                            }
                        }

                        if (_xi_changes_len>0) {
                            DBG(256) {
                                logu_("DirectInput:: number of input changes: %d\n", _xi_changes_len);
                                for (int i=0; i<_xi_changes_len; i++) {
                                    logu_("DirectInput:: change: what=%s, state=%d\n", _xi_utf8_names[_xi_changes[i].what], _xi_changes[i].state);
                                }
                            }
                            // lua callback: generate input-change event
                            module_t *m = *_curr_overlay_m;
                            module_gamepad_input(m, _xi_changes, _xi_changes_len);
                        }
                    }
                }
                else {
                    if (hr == DIERR_INVALIDPARAM) {
                        logu_("failed to get device state: DIERR_INVALIDPARAM\n");
                    }
                    else if (hr == DIERR_NOTACQUIRED) {
                        logu_("failed to get device state: DIERR_NOTACQUIRED\n");
                    }
                    else {
                        logu_("failed to get device state: %x\n", hr);
                    }
                }
                g_IDirectInputDevice8->Unacquire();
            }

            if (!input_processing_done) {
                di_fail_count = di_fail_count + 1;
            }
        }

        if (di_skip_count > 0) {
            di_skip_count--;
        }
        if (di_fail_count >= 5) {
            di_fail_count--;
            di_skip_count = skips;
        }

        set_controller_poll_delay();

        DBG(512) log_(L"Sleep for %d msec\n", _controller_poll_delay);
        Sleep(_controller_poll_delay);
    }
    logu_("Done polling DirectInput device\n");
    return 0;
}

bool siderVirtualProtect(void *addr, size_t size, DWORD newProt, DWORD *oldProt) {
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery(addr, &mbi, sizeof(mbi)) == sizeof(mbi)) {
        logu_("Memory info: state=0x%x, protect=0x%x, type=0x%x\n", mbi.State, mbi.Protect, mbi.Type);
        return VirtualProtect(addr, size, newProt, oldProt);
    }
    logu_("VirtualQuery failed for %p (error_code: 0x%x)\n", addr, GetLastError());
    return false;
}

void HookVtblMethod(void *self, int idx, map<BYTE**,BYTE*>* vtables, void *new_func, char *name)
{
    lock_t lock(&_cs);
    if (!self) {
        logu_("WARN: self is null. Nothing to hook");
        return;
    }

    BYTE** vtbl = *(BYTE***)self;
    BYTE *f = vtbl[idx];

    //DBG(64) logu_("current %s = %p\n", name, f);
    if ((BYTE*)f == (BYTE*)new_func) {
        //DBG(64) logu_("%s already hooked.\n", name);
    }
    else {
        logu_("Hooking %s\n", name);
        vtables->insert(pair<BYTE**,BYTE*>(vtbl, f));
        logu_("org %s = %p\n", name, f);

        DWORD protection = 0;
        DWORD newProtection = PAGE_EXECUTE_READWRITE;
        if (siderVirtualProtect(vtbl+idx, 8, newProtection, &protection) || (_config->_skip_checks & 0x04)) {
            vtbl[idx] = (BYTE*)new_func;
            f = vtbl[idx];
            logu_("now %s = %p\n", name, f);
        }
        else {
            logu_("ERROR: VirtualProtect failed for: %p\n", vtbl+idx);
        }
    }
}

void HookVtblMethod2(void *self, int idx, void **old_func, void **old_func2, void *new_func, void *new_func2, char *name)
{
    lock_t lock(&_cs);
    if (!self) {
        logu_("WARN: self is null. Nothing to hook\n");
        return;
    }

    BYTE** vtbl = *(BYTE***)self;
    BYTE *f = vtbl[idx];

    if ((BYTE*)f == (BYTE*)new_func) {
        logu_("%s already hooked to %p.\n", name, new_func);
    }
    else {
        logu_("Hooking %s\n", name);
        if ((BYTE*)f == (BYTE*)new_func2) {
            *old_func = *old_func2;
        }
        else {
            *old_func = f;
        }
        logu_("old_func is now: %p\n", *old_func);
        logu_("org %s = %p\n", name, f);

        DWORD protection = 0;
        DWORD newProtection = PAGE_EXECUTE_READWRITE;
        if (siderVirtualProtect(vtbl+idx, 8, newProtection, &protection) || (_config->_skip_checks & 0x08)) {
            vtbl[idx] = (BYTE*)new_func;
            f = vtbl[idx];
            logu_("now %s = %p\n", name, f);
        }
        else {
            logu_("ERROR: VirtualProtect failed for: %p\n", vtbl+idx);
        }
    }
}

HRESULT sider_CreateDevice(IDirectInput8 *self, REFGUID rguid, LPDIRECTINPUTDEVICE * lplpDirectInputDevice, LPUNKNOWN pUnkOuter)
{
    logu_("****\nsider_CreateDevice(self:%p): called\n", self);
    /**
    map<BYTE**,BYTE*>::iterator it = _vtables.find(*(BYTE***)self);
    if (it == _vtables.end()) {
        // bad
        logu_("unable to find vtable entry for self: %p\n");
        return S_OK;
    }
    BYTE *f = it->second;
    PFN_IDirectInput8_CreateDevice org_f = (PFN_IDirectInput8_CreateDevice)f;
    HRESULT res = org_f(self, rguid, lplpDirectInputDevice, pUnkOuter);
    **/
    HRESULT res = _org_CreateDevice(self, rguid, lplpDirectInputDevice, pUnkOuter);

    logu_("IDirectInput8::CreateDevice(%p, %p, %p, %p) returned: %p\n", self, rguid, lplpDirectInputDevice, pUnkOuter, *lplpDirectInputDevice);
    wchar_t guid_str[256];
    if (StringFromGUID2(rguid, guid_str, 256)) {
        log_(L"rguid: %s\n", guid_str);
    }
    if (rguid == GUID_SysKeyboard) {
        logu_("this is a keyboard device (GUID_SysKeyboard): %p\n", *lplpDirectInputDevice);
        HookVtblMethod2(
            *lplpDirectInputDevice, 9,
            (void**)&_org_GetDeviceStateKeyboard, (void**)&_org_GetDeviceStateGamepad,
            sider_GetDeviceStateKeyboard, sider_GetDeviceStateGamepad,
            "IDirectInputDevice8::GetDeviceState");
    }
    else if (rguid == GUID_SysMouse) {
        logu_("this is a mouse device (GUID_SysMouse): %p\n", *lplpDirectInputDevice);
    }
    else if (rguid == GUID_Joystick) {
        logu_("this is a joystick device (GUID_Joystick): %p\n", *lplpDirectInputDevice);
    }
    else if (rguid == GUID_SysMouseEm) {
        logu_("this is a mouse device (GUID_SysMouseEm): %p\n", *lplpDirectInputDevice);
    }
    else if (rguid == GUID_SysMouseEm2) {
        logu_("this is a mouse device (GUID_SysMouseEm2): %p\n", *lplpDirectInputDevice);
    }
    else if (rguid == GUID_SysKeyboardEm) {
        logu_("this is a keyboard device (GUID_SysKeyboardEm): %p\n", *lplpDirectInputDevice);
        if (_config->_hook_all_keyboards) {
            HookVtblMethod2(
                *lplpDirectInputDevice, 9,
                (void**)&_org_GetDeviceStateKeyboard, (void**)&_org_GetDeviceStateGamepad,
                sider_GetDeviceStateKeyboard, sider_GetDeviceStateGamepad,
                "IDirectInputDevice8::GetDeviceState");
        }
    }
    else if (rguid == GUID_SysKeyboardEm2) {
        logu_("this is a keyboard device (GUID_SysKeyboardEm2): %p\n", *lplpDirectInputDevice);
        if (_config->_hook_all_keyboards) {
            HookVtblMethod2(
                *lplpDirectInputDevice, 9,
                (void**)&_org_GetDeviceStateKeyboard, (void**)&_org_GetDeviceStateGamepad,
                sider_GetDeviceStateKeyboard, sider_GetDeviceStateGamepad,
                "IDirectInputDevice8::GetDeviceState");
        }
    }
    else {
        logu_("this is some other device: %p\n", *lplpDirectInputDevice);
        if (_config->_controller_input_blocking_enabled) {
            HookVtblMethod2(
                *lplpDirectInputDevice, 9,
                (void**)&_org_GetDeviceStateGamepad, (void**)&_org_GetDeviceStateKeyboard,
                sider_GetDeviceStateGamepad, sider_GetDeviceStateKeyboard,
                "IDirectInputDevice8::GetDeviceState");
        }
        else {
            logu_("gamepad-input-blocking is disabled. Nothing to do here\n");
        }
    }
    return res;
}

HRESULT sider_GetDeviceStateGamepad(IDirectInputDevice8 *self, DWORD cbData, LPVOID lpvData)
{
    DBG(16384) logu_("sider_GetDeviceStateGamepad(self:%p): called\n", self);
    HRESULT res = _org_GetDeviceStateGamepad(self, cbData, lpvData);
    if (_overlay_on && (_config->_global_block_input || _block_input)) {
        if (self != g_IDirectInputDevice8) {
            // block input to game
            return DIERR_INPUTLOST;
        }
    }
    DBG(16384) logu_("sider_GetDeviceStateGamepad(self:%p): res = %x\n", self, res);
    return res;
}

HRESULT sider_GetDeviceStateKeyboard(IDirectInputDevice8 *self, DWORD cbData, LPVOID lpvData)
{
    DBG(16384) logu_("sider_GetDeviceStateKeyboard(self:%p): called\n", self);
    HRESULT res = _org_GetDeviceStateKeyboard(self, cbData, lpvData);
    if (_overlay_on && (_config->_global_block_input || _block_input)) {
        // still check for our gamepad interface, because it is possible
        // that both types of devices are funneled through one intermediate
        if (self != g_IDirectInputDevice8) {
            // block input to game
            return DIERR_INPUTLOST;
        }
    }
    DBG(16384) logu_("sider_GetDeviceStateKeyboard(self:%p): res = %x\n", self, res);
    return res;
}

DWORD sider_XInputGetState(DWORD dwUserIndex, XINPUT_STATE *pState)
{
    DBG(16384) logu_("sider_XInputGetState(dwUserIndex:%x, pState:%p): called\n", dwUserIndex, pState);
    if (_overlay_on && (_config->_global_block_input || _block_input)) {
        // block input to game
        return ERROR_SUCCESS;
    }
    DWORD res = _org_XInputGetState(dwUserIndex, pState);
    return res;
}

void HookXInputGetState()
{
    log_(L"XInputGetState: %p\n", XInputGetState);
    BYTE *jmp_xinput_get_state = get_target_location2(_config->_hp_at_xinput);
    if (jmp_xinput_get_state) {
        _xinput_get_state_holder = (BYTE**)get_target_location(jmp_xinput_get_state);
        log_(L"xinput_get_state_holder: %p\n", _xinput_get_state_holder);
        if (_xinput_get_state_holder) {
            _org_XInputGetState = (PFN_XInputGetState)(*_xinput_get_state_holder);
            log_(L"_org_XInputGetState: %p\n", _org_XInputGetState);

            DWORD protection;
            DWORD newProtection = PAGE_EXECUTE_READWRITE;
            if (siderVirtualProtect(_xinput_get_state_holder, 8, newProtection, &protection) || (_config->_skip_checks & 0x10)) {
                *_xinput_get_state_holder = (BYTE*)sider_XInputGetState;
                log_(L"now XInputGetState: %p\n", *_xinput_get_state_holder);
            }
            else {
                log_(L"ERROR: VirtualProtect failed for: %p\n", _xinput_get_state_holder);
            }
        }
    }
}

HRESULT sider_Present(IDXGISwapChain *swapChain, UINT SyncInterval, UINT Flags)
{
    //logu_("Present called for swapChain: %p\n", swapChain);
    //logu_("Present:: gettop: %d\n", lua_gettop(L));

    if (kb_handle == NULL && _config->_overlay_enabled) {
        kb_handle = SetWindowsHookEx(WH_KEYBOARD, sider_keyboard_proc, NULL, GetCurrentThreadId());
        logu_("kb_handle = %p\n", kb_handle);
    }

    if (!_enumerated_controllers) {
        enumerate_controllers();
    }

    if (_reload_modified) {
        clear_overlay_texture();
        lua_reload_modified_modules();
        _reload_modified = false;
        _reload_1_down = false;
    }

    // process priority
    if (!_priority_set) {
        _priority_set = true;
        if (_config->_priority_class) {
            if (SetPriorityClass(GetCurrentProcess(), _config->_priority_class)) {
                logu_("SetPriorityClass successful for priority: 0x%x\n", _config->_priority_class);
            }
            else {
                logu_("SetPriorityClass failed for priority: 0x%x\n", _config->_priority_class);
            }
        }
    }

    if (!_controller_poll_initialized) {
        _controller_poll_initialized = true;
        if (_gamepad_config->_dinput_enabled || _gamepad_config->_xinput_enabled) {
            _controller_poll = true;
            DWORD thread_id;
            _controller_poll_handle = CreateThread(NULL, 0, direct_input_poll, NULL, 0, &thread_id);
            SetThreadPriority(_controller_poll_handle, THREAD_PRIORITY_LOWEST);
            logu_("created controller poll thread: 0x%x\n", thread_id);
        }
    }

    if (_has_controller) {
        if (!_controller_prepped) {
            HRESULT hr;
            if (FAILED(g_IDirectInputDevice8->SetCooperativeLevel(DX11.Window, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE))) {
                logu_("failed to set cooperative level\n");
            }
            hr = g_IDirectInputDevice8->SetDataFormat(&_data_format);
            if (FAILED(hr)) {
                if (hr == DIERR_INVALIDPARAM) {
                    logu_("failed to set data format: DIERR_INVALIDPARAM\n");
                }
                else {
                    logu_("failed to set data format: %x\n");
                }
            }
            _controller_prepped = true;
        }
    }

    if (_has_on_frame) {
        if (_config->_lua_enabled) {
            // lua call-backs
            vector<module_t*>::iterator i;
            for (i = _modules.begin(); i != _modules.end(); i++) {
                module_t *m = *i;
                module_on_frame(m);
            }
        }
    }

    if (_overlay_on) {
        // ask currently active module for text
        char *text = NULL;
        if (_config->_lua_enabled) {
            // lua callbacks
            if (_curr_overlay_m != _modules.end()) {
                char *image_path = NULL;
                layout_t opts;
                memset(&opts, 0, sizeof(layout_t));
                int image_width = 0;
                int image_hmargin;
                int image_vmargin;
                module_t *m = *_curr_overlay_m;
                module_overlay_on(m, &text, &image_path, &opts);
                if (text) {
                    wchar_t *ws = Utf8::utf8ToUnicode(text);
                    wcscpy(_current_overlay_text, ws);
                    Utf8::free(ws);
                }
                else {
                    // empty
                    _current_overlay_text[0] = L'\0';
                }

                if (_overlay_image.to_clear) {
                    clear_overlay_texture();
                }

                if (image_path != NULL) {
                    if (!_overlay_image.filepath || strcmp(image_path, _overlay_image.filepath)!=0) {
                        // load image into texture
                        SAFE_RELEASE(g_texture);
                        SAFE_RELEASE(g_textureView);
                        _overlay_image.filepath = strdup(image_path);

                        HRESULT hr;
                        wchar_t *ws = Utf8::utf8ToUnicode(image_path);
                        if (memcmp(".dds", image_path+strlen(image_path)-4, 4)==0) {
                            hr = DirectX::CreateDDSTextureFromFile(DX11.Device, ws, &g_texture, &g_textureView);
                        }
                        else {
                            // try other supported formats
                            hr = DirectX::CreateWICTextureFromFile(DX11.Device, ws, &g_texture, &g_textureView);
                        }
                        Utf8::free(ws);
                        if (SUCCEEDED(hr)) {
                            DBG(128) logu_("Loaded 2D texture: {%s}\n", _overlay_image.filepath);
                            _overlay_image.have = true;

                            D3D11_RESOURCE_DIMENSION resType = D3D11_RESOURCE_DIMENSION_UNKNOWN;
                            g_texture->GetType( &resType );

                            switch( resType )
                            {
                            case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
                                {
                                    ID3D11Texture2D* tex = (ID3D11Texture2D*)g_texture;
                                    D3D11_TEXTURE2D_DESC desc;
                                    tex->GetDesc(&desc);

                                    // This is a 2D texture. Check values of desc here
                                    DBG(128) logu_("texture Width: %d\n", desc.Width);
                                    DBG(128) logu_("texture Height: %d\n", desc.Height);
                                    DBG(128) logu_("texture MipLevels: %d\n", desc.MipLevels);
                                    DBG(128) logu_("texture ArraySize: %d\n", desc.ArraySize);
                                    DBG(128) logu_("texture Format: %d\n", desc.Format);
                                    _overlay_image.source_width = desc.Width;
                                    _overlay_image.source_height = desc.Height;

                                    // calculate dimensions based on two of:
                                    // opts.image_width, opts.image_height, opts.image_aspect_ratio
                                    // if not enough info: use default width of 0.1*screen-width and source image aspect ratio
                                    if (opts.image_width > 0) {
                                        _overlay_image.width = opts.image_width;
                                        if (opts.image_height > 0) {
                                            _overlay_image.height = opts.image_height;
                                        }
                                        else if (opts.has_image_ar) {
                                            _overlay_image.height = _overlay_image.width / opts.image_aspect_ratio;
                                        }
                                        else {
                                            _overlay_image.height = _overlay_image.width * ((double)desc.Height / desc.Width);
                                        }
                                    }
                                    else {
                                        // width is not specified
                                        if (opts.image_height > 0) {
                                            _overlay_image.height = opts.image_height;
                                            if (opts.has_image_ar) {
                                                _overlay_image.width = _overlay_image.height * opts.image_aspect_ratio;
                                            }
                                            else {
                                                _overlay_image.width = _overlay_image.height * ((double)desc.Width / desc.Height);
                                            }
                                        }
                                        else {
                                            // neither width nor height specified
                                            _overlay_image.width = min(desc.Width, DX11.Width * 0.1);
                                            if (opts.has_image_ar) {
                                                _overlay_image.height = _overlay_image.width / opts.image_aspect_ratio;
                                            }
                                            else {
                                                _overlay_image.height = _overlay_image.width * ((double)desc.Height / desc.Width);
                                            }
                                        }
                                    }

                                    _overlay_image.hmargin = (opts.has_image_hmargin) ? opts.image_hmargin : 10.0f;
                                    _overlay_image.vmargin = (opts.has_image_vmargin) ? opts.image_vmargin : 10.0f;
                                    DBG(128) logu_("on-screen pixels-width: %d\n", _overlay_image.width);
                                    DBG(128) logu_("on-screen pixels-height: %d\n", _overlay_image.height);
                                }
                                break;
                            default:
                                logu_("PROBLEM: Not a 2D texture: {%s}\n", _overlay_image.filepath);
                                if (_overlay_image.filepath) { free(_overlay_image.filepath); }
                                _overlay_image.have = false;
                            }
                        }
                        else {
                            logu_("PROBLEM: Cannot load texture from: {%s}\n", _overlay_image.filepath);
                            _overlay_image.have = false;
                        }
                    }
                }
                else {
                    // image_path is NULL, so clear the image
                    clear_overlay_texture();
                }

                float right_margin = (_overlay_image.have) ? _overlay_image.width + _overlay_image.hmargin*2 : 5.0f;

                // render overlay
                DX11.Device->GetImmediateContext(&DX11.Context);
                int pixel_height = prep_ui(_font_size, right_margin);
                if (pixel_height > 0) {
                    float top = (_config->_overlay_location == 0) ? 0 : DX11.Height - pixel_height;
                    draw_ui(top, top + pixel_height, right_margin);
                }
                SAFE_RELEASE(g_pVertexBuffer);
                SAFE_RELEASE(g_pTexVertexBuffer);
                DX11.Context->Release();
            }
        }
    }

    HRESULT hr = _org_Present(swapChain, SyncInterval, Flags);
    return hr;
}

HRESULT sider_CreateSwapChain(IDXGIFactory1 *pFactory, IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC *pDesc, IDXGISwapChain **ppSwapChain)
{
    HRESULT hr = _org_CreateSwapChain(pFactory, pDevice, pDesc, ppSwapChain);
    logu_("hr=0x%x, IDXGISwapChain: %p\n", hr, *ppSwapChain);
    _swap_chain = *ppSwapChain;

    _device = (ID3D11Device*)pDevice;
    logu_("==> device: %p\n", _device);
    if (SUCCEEDED(_swap_chain->GetDevice(__uuidof(ID3D11Device), (void**)&_device))) {
        logu_("==> device: %p\n", _device);
    }

    _device_context = NULL;
    _device->GetImmediateContext(&_device_context);
    logu_("==> device context: %p\n", _device_context);
    logu_("==> swap chain: %p\n", _swap_chain);
    if (_device_context) {
        _device_context->Release();
    }

    DX11.Device = _device;
    DX11.SwapChain = _swap_chain;
    DX11.Device->GetImmediateContext(&DX11.Context);

    DXGI_SWAP_CHAIN_DESC desc;
    if (SUCCEEDED(DX11.SwapChain->GetDesc(&desc))) {
        DX11.Window = desc.OutputWindow;
        DX11.Width = desc.BufferDesc.Width;
        DX11.Height = desc.BufferDesc.Height;
        logu_("==> window handle: %p\n", DX11.Window);
    }

    // check if we need to hook Present method
    IDXGISwapChain *sc = (IDXGISwapChain*)(*ppSwapChain);
    BYTE** vtbl = *(BYTE***)sc;
    PFN_IDXGISwapChain_Present present = (PFN_IDXGISwapChain_Present)vtbl[8];
    DBG(64) logu_("current Present = %p\n", present);
    if ((BYTE*)present == (BYTE*)sider_Present) {
        DBG(64) logu_("Present already hooked.\n");
    }
    else {
        prep_stuff();

        logu_("Hooking Present\n");
        _org_Present = present;
        logu_("_org_Present = %p\n", _org_Present);

        DWORD protection = 0;
        DWORD newProtection = PAGE_EXECUTE_READWRITE;
        if (siderVirtualProtect(vtbl+8, 8, newProtection, &protection) || (_config->_skip_checks & 0x02)) {
            vtbl[8] = (BYTE*)sider_Present;

            present = (PFN_IDXGISwapChain_Present)vtbl[8];
            logu_("now Present = %p\n", present);
        }
        else {
            logu_("ERROR: VirtualProtect failed for: %p\n", vtbl+8);
        }
    }

    return hr;
}

HRESULT sider_CreateDXGIFactory1(REFIID riid, void **ppFactory)
{
    HRESULT hr = _org_CreateDXGIFactory1(riid, ppFactory);
    DBG(64) logu_("hr=0x%x, IDXGIFactory1: %p\n", hr, *ppFactory);

    // check if we need to hook SwapChain method
    IDXGIFactory1 *f = (IDXGIFactory1*)(*ppFactory);
    BYTE** vtbl = *(BYTE***)f;
    PFN_IDXGIFactory1_CreateSwapChain sc = (PFN_IDXGIFactory1_CreateSwapChain)vtbl[10];
    DBG(64) logu_("current CreateSwapChain = %p\n", sc);
    if ((BYTE*)sc == (BYTE*)sider_CreateSwapChain) {
        DBG(64) logu_("CreateSwapChain already hooked.\n");
    }
    else {
        logu_("Hooking CreateSwapChain\n");
        _org_CreateSwapChain = sc;
        logu_("_org_CreateSwapChain = %p\n", _org_CreateSwapChain);

        DWORD protection = 0;
        DWORD newProtection = PAGE_EXECUTE_READWRITE;
        if (siderVirtualProtect(vtbl+10, 8, newProtection, &protection) || (_config->_skip_checks & 0x01)) {
            vtbl[10] = (BYTE*)sider_CreateSwapChain;

            sc = (PFN_IDXGIFactory1_CreateSwapChain)vtbl[10];
            logu_("now CreateSwapChain = %p\n", sc);
        }
        else {
            logu_("ERROR: VirtualProtect failed for: %p\n", vtbl+10);
        }
    }
    return hr;
}

BOOL sider_read_file(
    HANDLE       hFile,
    LPVOID       lpBuffer,
    DWORD        nNumberOfBytesToRead,
    LPDWORD      lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped,
    struct READ_STRUCT *rs)
{
    BOOL result;
    HANDLE orgHandle = hFile;
    DWORD orgBytesToRead = nNumberOfBytesToRead;
    HANDLE handle = INVALID_HANDLE_VALUE;
    FILE_LOAD_INFO *fli;

    //log_(L"rs (R12) = %p\n", rs);
    if (rs && !IsBadReadPtr(rs, sizeof(struct READ_STRUCT))) {
        BYTE* p = (BYTE*)rs;
        fli = (FILE_LOAD_INFO *)p;
        //DBG(3) logu_("read_file:: fli = %p\n", fli);

        wchar_t *fn;
        { PERF_TIMER(_fileops_stats); if (!have_cached(rs->filename, &fn)) {
            if (_config->_lua_enabled && _rewrite_count > 0) do_rewrite(rs->filename);
            DBG(1) logu_("read_file:: rs->filesize: %llx, rs->offset: %llx, rs->filename: %s\n",
                rs->filesize, rs->offset.full, rs->filename);
            fn = (_config->_lua_enabled) ? have_content(rs->filename) : NULL;
            fn = (fn) ? fn : have_live_file(rs->filename);
            cache_it(rs->filename, fn);
        }}
        if (fn != NULL) {
            DBG(3) log_(L"read_file:: livecpk file found: %s\n", fn);
            handle = CreateFileW(fn,         // file to open
                               GENERIC_READ,          // open for reading
                               FILE_SHARE_READ,       // share for reading
                               NULL,                  // default security
                               OPEN_EXISTING,         // existing file only
                               FILE_ATTRIBUTE_NORMAL, // normal file
                               NULL);                 // no attr. template

            if (handle != INVALID_HANDLE_VALUE)
            {
                DWORD sz = GetFileSize(handle, NULL);

                // replace file handle
                orgHandle = hFile;
                hFile = handle;

                // set correct offset
                LONG offsetHigh = rs->offset.parts.high;
                SetFilePointer(hFile, rs->offset.parts.low, &offsetHigh, FILE_BEGIN);
                rs->offset.parts.high = offsetHigh;
                LONGLONG offset = rs->offset.full;

                if (fli && !IsBadReadPtr(fli, sizeof(struct FILE_LOAD_INFO))) {
                    // adjust offset for multi-part reads
                    SetFilePointer(hFile, fli->bytes_read_so_far, NULL, FILE_CURRENT);
                    offset = offset + fli->bytes_read_so_far;

                    // trace file read info
                    DBG(4) {
                        logu_("read_file:: fli->total_bytes_to_read: %x\n", fli->total_bytes_to_read);
                        logu_("read_file:: fli->max_bytes_to_read: %x\n", fli->max_bytes_to_read);
                        logu_("read_file:: fli->bytes_to_read: %x\n", fli->bytes_to_read);
                        logu_("read_file:: fli->bytes_read_so_far: %x\n", fli->bytes_read_so_far);
                        logu_("read_file:: fli->filesize: %llx\n", fli->filesize);
                        logu_("read_file:: fli->buffer_size: %llx\n", fli->buffer_size);
                        logu_("read_file:: fli->cpk_filename: %s\n", fli->cpk_filename);
                        logu_("read_file:: fli->offset_in_cpk: %llx\n", fli->offset_in_cpk);
                    }
                }

                DBG(3) log_(L"read_file:: livecpk file offset: %llx\n", offset);
            }
        }
    }

    result = ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
    //log_(L"ReadFile(%x, %p, %x, %x, %p)\n",
    //    hFile, lpBuffer, nNumberOfBytesToRead, *lpNumberOfBytesRead, lpOverlapped);

    LONGLONG num_bytes_read = *lpNumberOfBytesRead;
    num_bytes_read = 0x00000000ffffffffL & num_bytes_read;

    if (handle != INVALID_HANDLE_VALUE) {
        DBG(3) log_(L"read_file:: called ReadFile(%x, %p, %x, %x, %p)\n",
            hFile, lpBuffer, nNumberOfBytesToRead, *lpNumberOfBytesRead, lpOverlapped);
        CloseHandle(handle);

        if (orgBytesToRead > *lpNumberOfBytesRead) {
            //log_(L"file-size adjustment: actually read = %x, reporting as read = %x\n",
            //    *lpNumberOfBytesRead, orgBytesToRead);
        }

        // fake a read from cpk
        if (orgBytesToRead > *lpNumberOfBytesRead) {
            *lpNumberOfBytesRead = orgBytesToRead;
        }
        //SetFilePointer(orgHandle, *lpNumberOfBytesRead, 0, FILE_CURRENT);
    }

    if (rs && !IsBadReadPtr(rs, sizeof(struct READ_STRUCT))) {
        // livecpk_read
        if (num_bytes_read > 0 && _config->_lua_enabled) {
            vector<module_t*>::iterator i;
            for (i = _modules.begin(); i != _modules.end(); i++) {
                module_t *m = *i;
                if (m->evt_lcpk_read != 0) {
                    module_read(m, rs->filename, lpBuffer, num_bytes_read, fli);
                }
            }
        }

        /**
        bool full_read = (fli) && (fli->bytes_read_so_far + num_bytes_read >= fli->total_bytes_to_read);
        if (full_read) {
            DBG(1024) logu_("full read of: %s, buffer=%p, buffer2=%p, sz=0x%x (lpBuffer=%p, bytes_read=0x%x)\n",
                rs->filename, fli->buffer, fli->buffer2, fli->total_bytes_to_read, lpBuffer, num_bytes_read);
        }
        **/
    }

    return result;
}

void sider_mem_copy(BYTE *dst, LONGLONG dst_len, BYTE *src, LONGLONG src_len, BYTE **rsp)
{
    HANDLE handle = INVALID_HANDLE_VALUE;
    wstring *filename = NULL;

    // do the original copy operation
    if (memcpy_s(dst, dst_len, src, src_len) != 0) {
        // don't try anything, if original call failed
        return;
    }

    LONGLONG dst_len_used = dst_len;

    // dump some stack
    //for (int i=0; i<(0x100/8); i++) {
    //    DBG(1) logu_("mem_copy:: %p (%x): %p\n", rsp+i, i, *(rsp+i));
    //}

    struct READ_STRUCT *rs = (struct READ_STRUCT*)*(rsp+0xf);
    if (rs) {
        BYTE* p = (BYTE*)rs;
        FILE_LOAD_INFO *fli = (FILE_LOAD_INFO *)p;
        //DBG(3) logu_("mem_copy:: fli = %p\n", fli);

        wchar_t *fn(NULL);
        { PERF_TIMER(_fileops_stats); if (!have_cached(rs->filename, &fn)) {
            if (_config->_lua_enabled && _rewrite_count > 0) do_rewrite(rs->filename);
            DBG(1) logu_("mem_copy:: rs->filesize: %llx, rs->offset: %llx, rs->filename: %s\n",
                rs->filesize, rs->offset.full, rs->filename);
            fn = (_config->_lua_enabled) ? have_content(rs->filename) : NULL;
            fn = (fn) ? fn : have_live_file(rs->filename);
            cache_it(rs->filename, fn);
        }}
        if (fn != NULL) {
            DBG(3) log_(L"mem_copy:: livecpk file found: %s\n", fn);
            handle = CreateFileW(fn,         // file to open
                               GENERIC_READ,          // open for reading
                               FILE_SHARE_READ,       // share for reading
                               NULL,                  // default security
                               OPEN_EXISTING,         // existing file only
                               FILE_ATTRIBUTE_NORMAL, // normal file
                               NULL);                 // no attr. template

            if (handle != INVALID_HANDLE_VALUE)
            {
                DWORD sz = GetFileSize(handle, NULL);

                // set correct offset
                LONG offsetHigh = rs->offset.parts.high;
                SetFilePointer(handle, rs->offset.parts.low, &offsetHigh, FILE_BEGIN);
                rs->offset.parts.high = offsetHigh;
                LONGLONG offset = rs->offset.full;

                if (fli && !IsBadReadPtr(fli, sizeof(struct FILE_LOAD_INFO))) {
                    // adjust offset for multi-part reads
                    SetFilePointer(handle, fli->bytes_read_so_far, NULL, FILE_CURRENT);
                    offset = offset + fli->bytes_read_so_far;

                    // trace file read info
                    DBG(4) {
                        logu_("mem_copy:: fli->total_bytes_to_read: %x\n", fli->total_bytes_to_read);
                        logu_("mem_copy:: fli->max_bytes_to_read: %x\n", fli->max_bytes_to_read);
                        logu_("mem_copy:: fli->bytes_to_read: %x\n", fli->bytes_to_read);
                        logu_("mem_copy:: fli->bytes_read_so_far: %x\n", fli->bytes_read_so_far);
                        logu_("mem_copy:: fli->filesize: %llx\n", fli->filesize);
                        logu_("mem_copy:: fli->buffer_size: %llx\n", fli->buffer_size);
                        logu_("mem_copy:: fli->cpk_filename: %s\n", fli->cpk_filename);
                        logu_("mem_copy:: fli->offset_in_cpk: %llx\n", fli->offset_in_cpk);
                    }
                }

                DBG(3) log_(L"mem_copy:: livecpk file offset: %llx\n", offset);

                // read data from file to destination buffer
                DWORD numberOfBytesRead = 0;
                BOOL result = ReadFile(handle, dst, src_len, &numberOfBytesRead, NULL);

                DBG(3) log_(L"mem_copy:: called ReadFile(%x, %p, %x, %x, %p)\n",
                    handle, dst, src_len, &numberOfBytesRead, NULL);
                CloseHandle(handle);

                dst_len_used = numberOfBytesRead;
            }
        }

        // livecpk_read
        if (_config->_lua_enabled) {
            vector<module_t*>::iterator i;
            for (i = _modules.begin(); i != _modules.end(); i++) {
                module_t *m = *i;
                if (m->evt_lcpk_read != 0) {
                    module_read(m, rs->filename, dst, dst_len_used, fli);
                }
            }
        }

        /**
        bool full_read = (fli) && (fli->bytes_read_so_far + dst_len_used >= fli->total_bytes_to_read);
        if (full_read) {
            DBG(1024) logu_("full read of: %s, buffer=%p, buffer2=%p, sz=0x%x (lpBuffer=%p, bytes_read=0x%x)\n",
                rs->filename, fli->buffer, fli->buffer2, fli->total_bytes_to_read, dst, dst_len_used);
        }
        **/
    }
}

void sider_lookup_file(LONGLONG p1, LONGLONG p2, char *filename)
{
    // quick check if we already modified this path
    size_t len = strlen(filename);
    char *p = filename + len + 1;
    if (*(DWORD*)p == MAGIC) {
        // already did this.
        return;
    }
    //DBG(8) logu_("lookup_file:: looking for: %s\n", filename);

    wchar_t *fn(NULL);
    { PERF_TIMER(_fileops_stats); if (!have_cached(filename, &fn)) {
        if (_config->_lua_enabled && _rewrite_count > 0) {
            if (do_rewrite(filename)) {
                len = strlen(filename);
                p = filename + len + 1;
            }
        }
        fn = (_config->_lua_enabled) ? have_content(filename) : NULL;
        fn = (fn) ? fn : have_live_file(filename);
        cache_it(filename, fn);
    }}
    if (fn) {
        DBG(4) logu_("lookup_file:: found livecpk file for: %s\n", filename);

        // trick: pick a filename that we know exists
        // put our filename after it, separated by MAGIC marker
        char temp[0x200];
        memcpy(temp, filename, len+1);
        memcpy(filename, _file_to_lookup, _file_to_lookup_size);
        memcpy(filename + _file_to_lookup_size, temp, len+1);
    }
    else {
        // not found. But still mark it with magic
        // so that we do not search again
        *(DWORD*)p = MAGIC;
        *(p+4) = '\0';
        *(p+5) = '\0';
    }
}

DWORD decode_team_id(DWORD team_id_encoded)
{
    return (team_id_encoded >> 0x0e) & 0x1ffff;
}

void sider_set_team_id(DWORD *dest, TEAM_INFO_STRUCT *team_info, DWORD offset)
{
    bool is_home = (offset == 0);
    DWORD *team_id_encoded = &(team_info->team_id_encoded);
    if (!dest || !team_id_encoded) {
        // safety check
        return;
    }

    if (is_home) {
        logu_("setting HOME team: %d\n", decode_team_id(*team_id_encoded));
        _home_team_info = team_info;
    }
    else {
        logu_("setting AWAY team: %d\n", decode_team_id(*team_id_encoded));
        _away_team_info = team_info;
    }

    BYTE *p = (BYTE*)dest - 0x138;
    p = (is_home) ? p : p - 0x690;
    MATCH_INFO_STRUCT *mi = (MATCH_INFO_STRUCT*)p;
    //logu_("mi: %p\n", mi);
    //logu_("mi->dw0: 0x%x\n", mi->dw0);
    if (!is_home) {
        logu_("tournament_id: %d\n", mi->tournament_id_encoded);
    }

    if (_config->_lua_enabled) {
        if (is_home) {
            lock_t lock(&_cs);
            clear_context_fields(_context_fields, _context_fields_count);
            _stadium_choice_count = 0;
            _sci = NULL;
        }
        else {
            _tournament_id = mi->tournament_id_encoded;
            set_context_field_int("tournament_id", mi->tournament_id_encoded);
            //set_context_field_int("match_time", mi->match_time);
            set_context_field_int("stadium_choice", mi->stadium_choice);
            set_match_info(mi);

            DWORD home = decode_team_id(*(DWORD*)((BYTE*)dest - 0x690));
            DWORD away = decode_team_id(*team_id_encoded);

            set_context_field_int("home_team", home);
            set_context_field_int("away_team", away);

            // lua call-backs
            vector<module_t*>::iterator i;
            for (i = _modules.begin(); i != _modules.end(); i++) {
                module_t *m = *i;
                module_set_teams(m, home, away); //, _home_team_info, _away_team_info);
            }
        }
    }
}

void sider_set_settings(STAD_STRUCT *dest_ss, STAD_STRUCT *src_ss)
{
    MATCH_INFO_STRUCT *mi = (MATCH_INFO_STRUCT*)((BYTE*)dest_ss - 0x70);
    bool ok = mi && (mi->num_subs >= 0 && mi->num_subs <= 11) && (mi->db0x17 >= 0x10 && mi->db0x17 < 0x20);
    if (!ok) {
        // safety check
        DBG(16) logu_("%02x %02x %02x\n", mi->num_subs, mi->num_subs_et, mi->db0x17);
        return;
    }
    DBG(16) logu_("%02x %02x %02x (ok)\n", mi->num_subs, mi->num_subs_et, mi->db0x17);

    logu_("tournament_id: %d\n", mi->tournament_id_encoded);

    if (_config->_lua_enabled) {
        // update match info
        //set_match_info(mi);

        // lua callbacks
        vector<module_t*>::iterator i;
        for (i = _modules.begin(); i != _modules.end(); i++) {
            module_t *m = *i;
            DWORD num_minutes = mi->match_time;
            if (module_set_match_time(m, &num_minutes)) {
                mi->match_time = num_minutes;
                //set_context_field_int("match_time", mi->match_time);
                break;
            }
        }
        for (i = _modules.begin(); i != _modules.end(); i++) {
            module_t *m = *i;
            if (module_set_stadium(m, mi)) {
                // sync the thumbnail
                mi->stadium_choice = mi->stad.stadium;
                break;
            }
        }
        for (i = _modules.begin(); i != _modules.end(); i++) {
            module_t *m = *i;
            if (module_set_conditions(m, mi)) {
                break;
            }
        }

        set_context_field_int("stadium", dest_ss->stadium);
        set_context_field_int("timeofday", dest_ss->timeofday);
        set_context_field_int("weather", dest_ss->weather);
        set_context_field_int("weather_effects", mi->weather_effects);
        set_context_field_int("season", dest_ss->season);

        for (i = _modules.begin(); i != _modules.end(); i++) {
            module_t *m = *i;
            if (module_set_match_settings(m, mi)) {
                break;
            }
        }

        set_context_field_int("difficulty", mi->difficulty);
        set_context_field_int("extra_time", mi->extra_time_choice);
        set_context_field_int("penalties", mi->penalties);
        set_context_field_int("substitutions", mi->num_subs);
        set_context_field_int("substitutions_in_extra_time", mi->num_subs_et);

        // clear stadium_choice in context
        //set_context_field_nil("stadium_choice");
        //_had_stadium_choice = false;

        for (i = _modules.begin(); i != _modules.end(); i++) {
            module_t *m = *i;
            module_after_set_conditions(m);
        }
    }

    logu_("settings now:: stadium=%d, timeofday=%d, weather=%d, season=%d\n",
        dest_ss->stadium, dest_ss->timeofday, dest_ss->weather, dest_ss->season);
}

DWORD sider_trophy_check(DWORD trophy_id)
{
    DWORD tid = trophy_id;
    DBG(16) logu_("trophy check:: trophy-id: 0x%0x\n", tid);
    if (_config->_lua_enabled) {
        // lua callbacks
        vector<module_t*>::iterator i;
        for (i = _modules.begin(); i != _modules.end(); i++) {
            module_t *m = *i;
            WORD new_tournament_id = _tournament_id;
            if (module_trophy_rewrite(m, _tournament_id, &new_tournament_id)) {
                EnterCriticalSection(&_tcs);
                for (int i=0; i<TT_LEN; i++) {
                    if (_trophy_map[i].tournament_id == new_tournament_id) {
                        DWORD new_tid = _trophy_map[i].trophy_id;
                        DBG(16) logu_("trophy check:: rewrite trophy-id: 0x%x --> 0x%x\n", tid, new_tid);
                        LeaveCriticalSection(&_tcs);
                        return new_tid;
                    }
                }
                LeaveCriticalSection(&_tcs);
            }
        }
    }
    return tid;
}

void sider_context_reset()
{
    lock_t lock(&_cs);
    clear_context_fields(_context_fields, _context_fields_count);
    _tournament_id = 0xffff;
    _stadium_choice_count = 0;
    _mi = NULL;
    _sci = NULL;
    _home_team_info = NULL;
    _away_team_info = NULL;

    logu_("context reset\n");

    if (_config->_lua_enabled) {
        // lua callbacks
        vector<module_t*>::iterator i;
        for (i = _modules.begin(); i != _modules.end(); i++) {
            module_context_reset(*i);
        }
    }
}

void sider_custom_event(uint16_t param, REGISTERS *regs) {
    if (_config->_lua_enabled) {
        // lua callbacks
        vector<module_t*>::iterator i;
        for (i = _modules.begin(); i != _modules.end(); i++) {
            bool processed = module_custom_event(*i, param, regs);
            if (processed) {
                break;
            }
        }
    }
}

void sider_free_select(BYTE *controller_restriction)
{
    *controller_restriction = 0;
}

void sider_trophy_table(TROPHY_TABLE_ENTRY *tt)
{
    //logu_("trophy table addr: %p\n", tt);
    EnterCriticalSection(&_tcs);
    memcpy(_trophy_map, tt, sizeof(_trophy_map));
    _trophy_table_copy_count++;
    LeaveCriticalSection(&_tcs);
}

char* sider_ball_name(char *ball_name)
{
    if (_config->_lua_enabled) {
        // lua callbacks
        vector<module_t*>::iterator i;
        for (i = _modules.begin(); i != _modules.end(); i++) {
            module_t *m = *i;
            char *new_ball_name = module_ball_name(m, ball_name);
            if (new_ball_name) {
                return new_ball_name;
            }
        }
    }
    return ball_name;
}

char* sider_stadium_name(STAD_INFO_STRUCT *stad_info, LONGLONG rdx, LONGLONG ptr, SCHEDULE_ENTRY *se)
{
    if (_config->_lua_enabled) {
        se = (ptr != 0 && ptr != 1) ? se : NULL;
        // lua callbacks
        vector<module_t*>::iterator i;
        for (i = _modules.begin(); i != _modules.end(); i++) {
            module_t *m = *i;
            char *new_stadium_name = module_stadium_name(m, stad_info->name, stad_info->id, se);
            if (new_stadium_name) {
                return new_stadium_name;
            }
        }
    }
    return stad_info->name;
}

STAD_INFO_STRUCT* sider_def_stadium_name(DWORD stadium_id)
{
    logu_("sider_def_stadium_name:: called for %d\n", stadium_id);
    memset(&_stadium_info, 0, sizeof(STAD_INFO_STRUCT));
    _stadium_info.id = stadium_id;
    strcpy((char*)&_stadium_info.name, "Unknown stadium");
    return &_stadium_info;
}

void sider_set_stadium_choice(MATCH_INFO_STRUCT *mi, WORD stadium_choice)
{
    _stadium_choice_count++;
    DBG(16) logu_("set_stadium_choice: mi->stadium_choice=%d, stadium_choice=%d\n", mi->stadium_choice, stadium_choice);
    bool just_updated = (mi->stadium_choice != stadium_choice);
    mi->stadium_choice = stadium_choice;
    if (!just_updated) {//_stadium_choice_count % 2 == 1) {
        if (_config->_lua_enabled) {
            // lua callbacks
            vector<module_t*>::iterator i;
            for (i = _modules.begin(); i != _modules.end(); i++) {
                module_t *m = *i;
                WORD new_stadium_choice;
                if (module_set_stadium_choice(m, stadium_choice, &new_stadium_choice)) {
                    mi->stadium_choice = new_stadium_choice;
                    break;
                }
            }
        }
        set_context_field_int("stadium_choice", mi->stadium_choice);
        logu_("set_stadium_choice: %d\n", mi->stadium_choice);
    }
}

DWORD sider_data_ready(FILE_LOAD_INFO *fli)
{
    if (!fli) {
        DBG(1024) logu_("sider_data_ready:: WARN: fli is NULL\n");
        return 0;
    }
    char *filename  = *(char**)((BYTE*)fli + 0x148);
    if (!filename) {
        DBG(1024) logu_("sider_data_ready:: WARN: filename is NULL\n");
        return 0;
    }
    LONGLONG base_offset_in_cpk  = *(LONGLONG*)((BYTE*)fli +  0x168);
    LONGLONG offs = fli->offset_in_cpk - base_offset_in_cpk;
    DBG(1024) logu_("sider_data_ready:: {%s}, {CPK:%s}, buffer=%p, size=0x%x, total=0x%x, sofar=0x%x, bsize=0x%x, offs=0x%x, type=%d\n",
        filename, fli->cpk_filename, fli->buffer, fli->filesize, fli->total_bytes_to_read,
        fli->bytes_read_so_far, fli->buffer_size, offs, fli->type);

    if (fli->type == 7) { // completely read and CRI-unpacked
        // livecpk_data_ready
        if (_config->_lua_enabled) {
            vector<module_t*>::iterator i;
            for (i = _modules.begin(); i != _modules.end(); i++) {
                module_t *m = *i;
                if (m->evt_lcpk_data_ready != 0) {
                    module_data_ready(m, filename, fli->buffer, fli->buffer_size, fli->filesize, offs, fli->cpk_filename);
                }
            }
        }
    }

    return 0;
}

void sider_kit_status(KIT_STATUS_INFO *ksi, TASK_UNIFORM_IMPL *tu_impl)
{
    _ksi = ksi;
    logu_("sider_kit_status: ksi=%p, tu_impl=%p\n", ksi, tu_impl);
    if (ksi) {
        logu_("sider_kit_status: ksi->home=%d, ksi->away=%d\n",
            decode_team_id(_ksi->home_team_id_encoded),
            decode_team_id(_ksi->away_team_id_encoded)
        );
        logu_("sider_kit_status: tu_impl->home=%d, tu_impl->away=%d\n",
            decode_team_id(tu_impl->home_team_id_encoded),
            decode_team_id(tu_impl->away_team_id_encoded)
        );
        logu_("sider_kit_status: is_edit_mode=%d\n", ksi->is_edit_mode);
    }
}

void sider_set_team_for_kits(KIT_STATUS_INFO *ksi, DWORD team_id_encoded, LONGLONG r8, DWORD *which)
{
    logu_("sider_set_team_for_kits: ksi=%p, which=%p\n", ksi, which);
    if (ksi && which) {
        if (which == &(ksi->home_team_id_encoded)) {
            logu_("sider_set_team_for_kits: home=%d, is_edit_mode=%d\n", decode_team_id(team_id_encoded), ksi->is_edit_mode);
            if (ksi->is_edit_mode) {
                set_context_field_int("edit_mode_team", decode_team_id(team_id_encoded));
                // reset context
                sider_context_reset();
            }
            if (_config->_lua_enabled) {
                // lua call-backs
                vector<module_t*>::iterator i;
                for (i = _modules.begin(); i != _modules.end(); i++) {
                    module_t *m = *i;
                    module_set_home_team_for_kits(m, decode_team_id(team_id_encoded), ksi->is_edit_mode);
                }
            }
        }
        else if (which == &(ksi->away_team_id_encoded)) {
            logu_("sider_set_team_for_kits: away=%d, is_edit_mode=%d\n", decode_team_id(team_id_encoded), ksi->is_edit_mode);

            if (_config->_lua_enabled) {
                // lua call-backs
                vector<module_t*>::iterator i;
                for (i = _modules.begin(); i != _modules.end(); i++) {
                    module_t *m = *i;
                    module_set_away_team_for_kits(m, decode_team_id(team_id_encoded), ksi->is_edit_mode);
                }
            }
        }
    }
}

void sider_clear_team_for_kits(KIT_STATUS_INFO *ksi, DWORD *which)
{
    if (ksi && which) {
        if (which == &(ksi->home_team_id_encoded)) {
            logu_("sider_clear_team_for_kits: home=%d, is_edit_mode=%d\n", decode_team_id(ksi->home_team_id_encoded), ksi->is_edit_mode);
            set_context_field_nil("edit_mode_team");
        }
        else if (which == &(ksi->away_team_id_encoded)) {
            logu_("sider_clear_team_for_kits: away=%d, is_edit_mode=%d\n", decode_team_id(ksi->away_team_id_encoded), ksi->is_edit_mode);
        }
    }
}

void sider_set_edit_team_id(DWORD team_id_encoded)
{
    DWORD edit_team_id = decode_team_id(team_id_encoded);
    if (_edit_team_id != edit_team_id) {
        _edit_team_id = edit_team_id;
        logu_("sider_edit_team_id: team = %d\n", _edit_team_id);
        if (_config->_lua_enabled) {
            if (_edit_team_id != 0x1ffff) {
                // set
                set_context_field_int("edit_mode_team", decode_team_id(team_id_encoded));
                set_context_field_int("home_team", decode_team_id(team_id_encoded));
            }
            else {
                // clear
                set_context_field_nil("edit_mode_team");
                set_context_field_nil("home_team");
            }
        }
    }
}

BYTE* sider_loaded_uniparam(BYTE* uniparam)
{
    size_t sz = *(size_t*)(uniparam-8);
    size_t new_sz;
    logu_("uniparam loaded at: %p\n", uniparam);
    logu_("uniparam size: %d (0x%x)\n", sz, sz);
    if (_config->_dummify_uniparam) {
        BYTE *new_uniparam = dummify_uniparam(uniparam, sz, &new_sz);
        if (new_uniparam) {
            logu_("new uniparam at: %p\n", new_uniparam);
            logu_("new uniparam size: %d (0x%x)\n", new_sz, new_sz);
            return new_uniparam;
        }
    }
    return uniparam;
}

void sider_clear_sc(SCOREBOARD_INFO *sci)
{
    lock_t lock(&_cs);
    logu_("clearing _sci: %p --> 0\n", sci);
    _sci = NULL;
}

void sider_check_kit_choice(MATCH_INFO_STRUCT *mi, DWORD home_or_away)
{
    logu_("check_kit_choice: mi=%p, home_or_away=%d\n", mi, home_or_away);
    if (home_or_away == 1) {
        return;
    }

    if (_config->_lua_enabled) {
        // lua call-backs
        vector<module_t*>::iterator i;
        for (i = _modules.begin(); i != _modules.end(); i++) {
            module_t *m = *i;
            if (module_set_kits(m, mi)) {
                break;
            }
        }
    }
}

BYTE* get_target_location(BYTE *call_location)
{
    if (call_location) {
        BYTE* bptr = call_location;
        DWORD protection = 0;
        DWORD newProtection = PAGE_EXECUTE_READWRITE;
        if (VirtualProtect(bptr, 8, newProtection, &protection)) {
            // get memory location where call target addr is stored
            // format of indirect call is like this:
            // call [addr] : FF 15 <4-byte-offset>
            DWORD* ptr = (DWORD*)(call_location + 2);
            VirtualProtect(bptr, 8, protection, &newProtection);
            return call_location + 6 + ptr[0];
        }
    }
    return NULL;
}

BYTE* get_target_location2(BYTE *rel_offs_location)
{
    if (rel_offs_location) {
        DWORD* ptr = (DWORD*)(rel_offs_location);
        return rel_offs_location + 4 + ptr[0];
    }
    return NULL;
}

void hook_indirect_call(BYTE *loc, BYTE *p) {
    if (!loc) {
        return;
    }
    DWORD protection = 0;
    DWORD newProtection = PAGE_EXECUTE_READWRITE;
    BYTE *addr_loc = get_target_location(loc);
    log_(L"loc: %p, addr_loc: %p\n", loc, addr_loc);
    if (VirtualProtect(addr_loc, 8, newProtection, &protection)) {
        BYTE** v = (BYTE**)addr_loc;
        *v = p;
        log_(L"hook_indirect_call: hooked at %p (target: %p)\n", loc, p);
    }
}

void move_code(BYTE *loc, int offset, size_t code_len) {
    if (!loc) {
        return;
    }
    DWORD protection = 0;
    DWORD newProtection = PAGE_EXECUTE_READWRITE;
    if (VirtualProtect(loc + offset, code_len, newProtection, &protection)) {
        memmove_s(loc + offset, code_len, loc, code_len);
        log_(L"move_code: moved %d bytes from %p to %p\n", code_len, loc, loc + offset);
    }
}

void hook_jmp(BYTE *loc, BYTE *p, size_t nops) {
    if (!loc) {
        return;
    }
    DWORD protection = 0;
    DWORD newProtection = PAGE_EXECUTE_READWRITE;
    if (VirtualProtect(loc, 12 + nops, newProtection, &protection)) {
        memcpy(loc, "\x48\xb8", 2);
        memcpy(loc+2, &p, sizeof(BYTE*));  // mov rax,<target_addr>
        memcpy(loc+10, "\xff\xe0", 2);      // jmp rax
        if (nops) {
            memset(loc+12, '\x90', nops);  // nop ;one of more nops for padding
        }
        log_(L"hook_jmp: hooked at %p (target: %p)\n", loc, p);
    }
}

void hook_call(BYTE *loc, BYTE *p, size_t nops) {
    if (!loc) {
        return;
    }
    DWORD protection = 0;
    DWORD newProtection = PAGE_EXECUTE_READWRITE;
    if (VirtualProtect(loc, 12 + nops, newProtection, &protection)) {
        memcpy(loc, "\x48\xb8", 2);
        memcpy(loc+2, &p, sizeof(BYTE*));  // mov rax,<target_addr>
        memcpy(loc+10, "\xff\xd0", 2);      // call rax
        if (nops) {
            memset(loc+12, '\x90', nops);  // nop ;one of more nops for padding
        }
        log_(L"hook_call: hooked at %p (target: %p)\n", loc, p);
    }
}

void hook_call_rcx(BYTE *loc, BYTE *p, size_t nops) {
    if (!loc) {
        return;
    }
    DWORD protection = 0;
    DWORD newProtection = PAGE_EXECUTE_READWRITE;
    if (VirtualProtect(loc, 12 + nops, newProtection, &protection)) {
        memcpy(loc, "\x48\xb9", 2);
        memcpy(loc+2, &p, sizeof(BYTE*));  // mov rcx,<target_addr>
        memcpy(loc+10, "\xff\xd1", 2);      // call rcx
        if (nops) {
            memset(loc+12, '\x90', nops);  // nop ;one of more nops for padding
        }
        log_(L"hook_call_rcx: hooked at %p (target: %p)\n", loc, p);
    }
}

void hook_call_rdx(BYTE *loc, BYTE *p, size_t nops) {
    if (!loc) {
        return;
    }
    DWORD protection = 0;
    DWORD newProtection = PAGE_EXECUTE_READWRITE;
    if (VirtualProtect(loc, 12 + nops, newProtection, &protection)) {
        memcpy(loc, "\x48\xba", 2);
        memcpy(loc+2, &p, sizeof(BYTE*));  // mov rcx,<target_addr>
        memcpy(loc+10, "\xff\xd2", 2);      // call rcx
        if (nops) {
            memset(loc+12, '\x90', nops);  // nop ;one of more nops for padding
        }
        log_(L"hook_call_rdx: hooked at %p (target: %p)\n", loc, p);
    }
}

void hook_call_with_tail(BYTE *loc, BYTE *p, BYTE *tail, size_t tail_size) {
    if (!loc) {
        return;
    }
    DWORD protection = 0;
    DWORD newProtection = PAGE_EXECUTE_READWRITE;
    if (VirtualProtect(loc, 12 + tail_size, newProtection, &protection)) {
        memcpy(loc, "\x48\xb8", 2);
        memcpy(loc+2, &p, sizeof(BYTE*));  // mov rax,<target_addr>
        memcpy(loc+10, "\xff\xd0", 2);      // call rax
        memcpy(loc+12, tail, tail_size);  // tail code
        log_(L"hook_call_with_tail: hooked at %p (target: %p)\n", loc, p);
    }
}

void hook_call_with_head_and_tail(BYTE *loc, BYTE *p, BYTE *head, size_t head_size, BYTE *tail, size_t tail_size) {
    if (!loc) {
        return;
    }
    DWORD protection = 0 ;
    DWORD newProtection = PAGE_EXECUTE_READWRITE;
    if (VirtualProtect(loc, head_size + 12 + tail_size, newProtection, &protection)) {
        memcpy(loc, head, head_size);   // head code
        memcpy(loc+head_size, "\x48\xb8", 2);
        memcpy(loc+head_size+2, &p, sizeof(BYTE*));  // mov rax,<target_addr>
        memcpy(loc+head_size+10, "\xff\xd0", 2);     // call rax
        memcpy(loc+head_size+12, tail, tail_size);   // tail code
        log_(L"hook_call_with_head_and_tail: hooked at %p (target: %p)\n", loc, p);
    }
}

void hook_call_rdx_with_head_and_tail(BYTE *loc, BYTE *p, BYTE *head, size_t head_size, BYTE *tail, size_t tail_size) {
    if (!loc) {
        return;
    }
    DWORD protection = 0 ;
    DWORD newProtection = PAGE_EXECUTE_READWRITE;
    if (VirtualProtect(loc, head_size + 12 + tail_size, newProtection, &protection)) {
        memcpy(loc, head, head_size);   // head code
        memcpy(loc+head_size, "\x48\xba", 2);
        memcpy(loc+head_size+2, &p, sizeof(BYTE*));  // mov rdx,<target_addr>
        memcpy(loc+head_size+10, "\xff\xd2", 2);     // call rdx
        memcpy(loc+head_size+12, tail, tail_size);   // tail code
        log_(L"hook_call_rdx_with_head_and_tail: hooked at %p (target: %p)\n", loc, p);
    }
}

void hook_call_rdx_with_head_and_tail_and_moved_call(BYTE *loc, BYTE *p, BYTE *head, size_t head_size, BYTE *tail, size_t tail_size, BYTE *moved_call_old, BYTE* moved_call_new) {
    if (!loc) {
        return;
    }
    DWORD protection = 0 ;
    DWORD newProtection = PAGE_EXECUTE_READWRITE;
    if (VirtualProtect(loc, head_size + 12 + tail_size + 0x100, newProtection, &protection)) {
        int old_call_offs = *(int*)(moved_call_old + 1);
        memcpy(loc, head, head_size);   // head code
        memcpy(loc+head_size, "\x48\xba", 2);
        memcpy(loc+head_size+2, &p, sizeof(BYTE*));  // mov rdx,<target_addr>
        memcpy(loc+head_size+10, "\xff\xd2", 2);     // call rdx
        memcpy(loc+head_size+12, tail, tail_size);   // tail code

        int new_call_offs = old_call_offs - (int)(moved_call_new - moved_call_old);
        *(int*)(moved_call_new + 1) = new_call_offs;

        log_(L"hook_call_rdx_with_head_and_tail_and_moved_call: hooked at %p (target: %p)\n", loc, p);
        //if (VirtualProtect(loc, head_size + 12 + tail_size, protection, &newProtection)) {
        //    log_(L"restored page protection at: %p\n", loc);
        //}
    }
}

static int get_team_id(MATCH_INFO_STRUCT *mi, int home_or_away)
{
    DWORD id_encoded = (home_or_away == 0) ? mi->home.team_id_encoded : mi->away.team_id_encoded;
    return decode_team_id(id_encoded);
}

static int sider_context_get_current_team_id(lua_State *L)
{
    if (!_ksi) {
        // no KIT_STATUS_INFO struct
        lua_pop(L, 1);
        return 0;
    }
    int home_or_away = luaL_checkinteger(L, 1);
    lua_pop(L, 1);
    DWORD team_id;
    switch (home_or_away) {
        case 0:
            team_id = decode_team_id(_ksi->home_team_id_encoded);
            lua_pushinteger(L, team_id);
            return 1;
        case 1:
            team_id = decode_team_id(_ksi->away_team_id_encoded);
            lua_pushinteger(L, team_id);
            return 1;
    }
    return 0;
}

static int sider_context_get_current_kit_id(lua_State *L)
{
    int home_or_away = luaL_checkinteger(L, 1);
    if (_mi) {
        lua_pop(L, 1);
        BYTE *kit = &((home_or_away==0) ? _mi->home_player_kit_id : _mi->away_player_kit_id);
        lua_pushinteger(L, *kit);
        return 1;
    }
    else if (_ksi && _ksi->task_uniform_impl) {
        if (home_or_away == 0 && _ksi->task_uniform_impl->home_team_id_encoded != 0xffffffff) {
            KIT_INTERMED_STRUCT *kis = _ksi->task_uniform_impl->home;
            if (kis && kis->kit_helper) {
                lua_pop(L, 1);
                if (kis->kit_helper->is_goalkeeper == 1) {
                    lua_pushinteger(L, 0);
                    lua_pushboolean(L, 1);
                    return 2;
                }
                else {
                    lua_pushinteger(L, _ksi->task_uniform_impl->home_player_kit_id);
                    lua_pushnil(L);
                    return 2;
                }
            }
        }
        if (home_or_away == 1 && _ksi->task_uniform_impl->away_team_id_encoded != 0xffffffff) {
            KIT_INTERMED_STRUCT *kis = _ksi->task_uniform_impl->away;
            if (kis && kis->kit_helper) {
                lua_pop(L, 1);
                if (kis->kit_helper->is_goalkeeper == 1) {
                    lua_pushinteger(L, 0);
                    lua_pushboolean(L, 1);
                    return 2;
                }
                else {
                    lua_pushinteger(L, _ksi->task_uniform_impl->away_player_kit_id);
                    lua_pushnil(L);
                    return 2;
                }
            }
        }
    }
    lua_pop(L, 1);
    return 0;
}

static int sider_context_set_current_kit_id(lua_State *L)
{
    if (!_mi) {
        // no match_info_struct
        lua_pop(L, 2);
        return 0;
    }
    int home_or_away = luaL_checkinteger(L, 1);
    int kit_id = luaL_checkinteger(L, 2);
    lua_pop(L, 2);
    BYTE *kit = &((home_or_away==0) ? _mi->home_player_kit_id : _mi->away_player_kit_id);
    *kit = kit_id;
    return 0;
}

static int sider_context_get_kit(lua_State *L)
{
    int team_id = luaL_checkinteger(L, 1);
    int kit_id = luaL_checkinteger(L, 2);
    if (kit_id > 9) { kit_id = 9; }
    if (kit_id < 0) { kit_id = 0; }

    DBG(2048) logu_("get_kit:: team_id=%d, kit_id=%d\n", team_id, kit_id);
    BYTE *src_data = find_kit_info(team_id, suffix_map[kit_id]);
    if (!src_data) {
        lua_pop(L, 2);
        return 0;
    }

    lua_pop(L, 2);

    lua_newtable(L);
    get_kit_info_to_lua_table(L, -1, src_data);

    // handle unicolors
    TEAM_INFO_STRUCT *ti = NULL;
    if (_mi) {
        if (decode_team_id(_mi->home.team_id_encoded) == team_id) {
            ti = &(_mi->home);
        }
        else if (decode_team_id(_mi->away.team_id_encoded) == team_id) {
            ti = &(_mi->away);
        }
        else if (_home_team_info && decode_team_id(_home_team_info->team_id_encoded) == team_id) {
            ti = _home_team_info;
        }
        else if (_away_team_info && decode_team_id(_away_team_info->team_id_encoded) == team_id) {
            ti = _away_team_info;
        }
    }
    if (ti) {
        BYTE *unicolor1 = (kit_id < 2) ? ti->players[kit_id].color1 : ti->extra_players[kit_id-2].color1;
        lua_pushfstring(L, "#%02x%02x%02x", unicolor1[0], unicolor1[1], unicolor1[2]);
        lua_setfield(L, -2, "UniColor_Color1");
        BYTE *unicolor2 = (kit_id < 2) ? ti->players[kit_id].color2 : ti->extra_players[kit_id-2].color2;
        lua_pushfstring(L, "#%02x%02x%02x", unicolor2[0], unicolor2[1], unicolor2[2]);
        lua_setfield(L, -2, "UniColor_Color2");
    }
    else {
        logu_("WARN: ti is unknown. Cannot determine unicolors\n");
    }

    return 1;
}

static int sider_context_get_gk_kit(lua_State *L)
{
    int team_id = luaL_checkinteger(L, 1);

    DBG(2048) logu_("get_gk_kit:: team_id=%d\n", team_id);
    BYTE *src_data = find_kit_info(team_id, gk_suffix_map[0]);
    if (!src_data) {
        lua_pop(L, 1);
        return 0;
    }

    lua_pop(L, 1);

    lua_newtable(L);
    get_kit_info_to_lua_table(L, -1, src_data);

    // handle unicolors
    TEAM_INFO_STRUCT *ti = NULL;
    if (_mi) {
        if (decode_team_id(_mi->home.team_id_encoded) == team_id) {
            ti = &(_mi->home);
        }
        else if (decode_team_id(_mi->away.team_id_encoded) == team_id) {
            ti = &(_mi->away);
        }
        else if (_home_team_info && decode_team_id(_home_team_info->team_id_encoded) == team_id) {
            ti = _home_team_info;
        }
        else if (_away_team_info && decode_team_id(_away_team_info->team_id_encoded) == team_id) {
            ti = _away_team_info;
        }
    }
    if (ti) {
        BYTE *unicolor1 = ti->goalkeepers[0].color1;
        lua_pushfstring(L, "#%02x%02x%02x", unicolor1[0], unicolor1[1], unicolor1[2]);
        lua_setfield(L, -2, "UniColor_Color1");
        BYTE *unicolor2 = ti->goalkeepers[0].color2;
        lua_pushfstring(L, "#%02x%02x%02x", unicolor2[0], unicolor2[1], unicolor2[2]);
        lua_setfield(L, -2, "UniColor_Color2");
    }
    else {
        logu_("WARN: ti is unknown. Cannot determine unicolors\n");
    }

    return 1;
}

static int sider_context_set_kit(lua_State *L)
{
    int team_id = luaL_checkinteger(L, 1);
    int kit_id = luaL_checkinteger(L, 2);
    if (kit_id > 9) { kit_id = 9; }
    if (kit_id < 0) { kit_id = 0; }

    // force refresh of kit
    if (lua_istable(L, 3)) {
        DBG(2048) logu_("set_kit:: team_id=%d, kit_id=%d\n", team_id, kit_id);
        DWORD len;
        BYTE *dst_data = find_kit_info(team_id, suffix_map[kit_id], &len);
        if (!dst_data) {
            logu_("problem: cannot find kit info for team %d, kit %d\n", team_id, kit_id);
            lua_pop(L, lua_gettop(L));
            return 0;
        }

        BYTE *radar_color = NULL;
        BYTE *shirt_color = NULL;

        if (lua_isnumber(L, 4)) {
            int home_or_away = luaL_checkinteger(L, 4);

            // if we are to apply radar, then we need to know: home or away
            if (_mi) {
                TEAM_INFO_STRUCT *ti = (home_or_away == 0) ? &(_mi->home) : &(_mi->away);
                radar_color = (kit_id < 2) ? ti->players[kit_id].color1 : ti->extra_players[kit_id-2].color1;
logu_("set_kit:: _mi ptr: %p\n", _mi);
logu_("set_kit:: _mi->home ptr: %p\n", &(_mi->home));
logu_("set_kit:: _mi->away ptr: %p\n", &(_mi->away));
logu_("set_kit:: ti ptr: %p\n", ti);
logu_("set_kit:: radar_color ptr: %p\n", radar_color);
            }
            else {
                logu_("warning: unable to set radar color - no MATCH_INFO_STRUCT pointer\n");
            }

            // also apply shirt colors, because in non-exhibition games
            // they are important for color-matching of kits during initial kit selection
            TEAM_INFO_STRUCT *ti = (home_or_away == 0) ? _home_team_info : _away_team_info;
            if (ti && decode_team_id(ti->team_id_encoded) == team_id) {
                SHIRTCOLOR_STRUCT *scs = NULL;
                if (kit_id < 2) {
                    scs = &(ti->players[kit_id]);
                }
                else {
                    scs = &(ti->extra_players[kit_id-2]);
                }

                shirt_color = scs->color1;
            }
            else {
                logu_("warning: unable to set shirt colors - team_id mismatch or team-info unknown\n");
            }
        }

        memset(dst_data, 0, len);
        set_kit_info_from_lua_table(L, 3, dst_data, radar_color, shirt_color);
    }
    lua_pop(L, lua_gettop(L));
    return 0;
}

static int sider_context_set_gk_kit(lua_State *L)
{
    int team_id = luaL_checkinteger(L, 1);

    // force refresh of kit
    if (lua_istable(L, 2)) {
        DBG(2048) logu_("set_gk_kit:: team_id=%d\n", team_id);
        DWORD len;
        BYTE *dst_data = find_kit_info(team_id, gk_suffix_map[0], &len);
        if (!dst_data) {
            logu_("problem: cannot find GK kit info for team %d\n", team_id);
            lua_pop(L, lua_gettop(L));
            return 0;
        }

        memset(dst_data, 0, len);
        set_kit_info_from_lua_table(L, 2, dst_data, NULL, NULL);
    }
    lua_pop(L, lua_gettop(L));
    return 0;
}

/**
static int sider_context_get_shirt_colors(lua_State *L)
{
    if (!lua_isuserdata(L, 1)) {
        lua_pop(L, lua_gettop(L));
        lua_pushstring(L, "first argument must be a lightuserdata pointer to team-info");
        return lua_error(L);
    }

    TEAM_INFO_STRUCT *team_info = (TEAM_INFO_STRUCT*)lua_touserdata(L, 1);
    int kit_id = luaL_checkinteger(L, 2);

    if (kit_id < 0 || kit_id > 9) {
        lua_pop(L, lua_gettop(L));
        lua_pushstring(L, "kit_id must be in [0,9] range");
        return lua_error(L);
    }

    lua_pop(L, lua_gettop(L));

    SHIRTCOLOR_STRUCT *scs = NULL;
    if (kit_id < 2) {
        scs = &(team_info->players[kit_id]);
    }
    else {
        scs = &(team_info->extra_players[kit_id-2]);
    }
    if (scs->index != 0x91) {
        lua_pushfstring(L, "#%02X%02X%02X", scs->color1[0], scs->color1[1], scs->color1[2]);
        lua_pushfstring(L, "#%02X%02X%02X", scs->color2[0], scs->color2[1], scs->color2[2]);
    }
    else {
        lua_pushnil(L);
        lua_pushnil(L);
    }
    return 2;
}

static int sider_context_set_shirt_colors(lua_State *L)
{
    if (!lua_isuserdata(L, 1)) {
        lua_pop(L, lua_gettop(L));
        lua_pushstring(L, "first argument must be a lightuserdata pointer to team-info");
        return lua_error(L);
    }

    TEAM_INFO_STRUCT *team_info = (TEAM_INFO_STRUCT*)lua_touserdata(L, 1);
    int kit_id = luaL_checkinteger(L, 2);

    if (kit_id < 0 || kit_id > 9) {
        lua_pop(L, lua_gettop(L));
        lua_pushstring(L, "kit_id must be in [0,9] range");
        return lua_error(L);
    }

    const char *color1 = luaL_checkstring(L, 3);
    const char *color2 = luaL_checkstring(L, 4);

    SHIRTCOLOR_STRUCT *scs = NULL;
    if (kit_id < 2) {
        scs = &(team_info->players[kit_id]);
    }
    else {
        scs = &(team_info->extra_players[kit_id-2]);
    }

    DWORD value;
    if (color1[0]!='\0') {
        char clr1[16] = {0};
        strcat(clr1,"0x");
        strncat(clr1,color1+1,6);
        if (sscanf(clr1, "%x", &value)==1) {
            scs->color1[0] = (value >> 16) & 0xff;
            scs->color1[1] = (value >> 8) & 0xff;
            scs->color1[2] = value & 0xff;
            logu_("color1 became: %02x,%02x,%02x\n", scs->color1[0], scs->color1[1], scs->color1[2]);
        }
    }
    if (color2[0]!='\0') {
        char clr2[16] = {0};
        strcat(clr2,"0x");
        strncat(clr2,color2+1,6);
        if (sscanf(clr2, "%x", &value)==1) {
            scs->color2[0] = (value >> 16) & 0xff;
            scs->color2[1] = (value >> 8) & 0xff;
            scs->color2[2] = value & 0xff;
            logu_("color2 became: %02x,%02x,%02x\n", scs->color2[0], scs->color2[1], scs->color2[2]);
        }
    }

    lua_pop(L, lua_gettop(L));
    return 0;
}
**/

static int sider_context_refresh_kit(lua_State *L)
{
    logu_("refresh_kit:: ~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    if (!_ksi || !_ksi->task_uniform_impl) {
        // no TASK_UNIFORM_IMPL object
        logu_("refresh_kit:: no ksi or no TaskUniformImpl\n");
        lua_pop(L, lua_gettop(L));
        return 0;
    }
    int home_or_away = luaL_checkinteger(L, 1);

    // more safety checks
    logu_("refresh_kit:: tu_impl=%p\n", _ksi->task_uniform_impl);
    if (home_or_away == 0 && _ksi->home_team_id_encoded == 0xffffffff) {
        logu_("refresh_kit:: no home team\n");
        lua_pop(L, lua_gettop(L));
        return 0;
    }
    if (home_or_away == 1 && _ksi->away_team_id_encoded == 0xffffffff) {
        logu_("refresh_kit:: no away team\n");
        lua_pop(L, lua_gettop(L));
        return 0;
    }

    KIT_INTERMED_STRUCT *kis = (home_or_away == 0) ?
        _ksi->task_uniform_impl->home : _ksi->task_uniform_impl->away;

    if (!kis || !kis->kit_helper) {
        // no intermed or kit-helper pointer
        logu_("refresh_kit:: no kis or no kis->kit_helper\n");
        lua_pop(L, lua_gettop(L));
        return 0;
    }

    logu_("refresh_kit:: kis=%p\n", kis);
    logu_("refresh_kit:: kis->kit_helper=%p\n", kis->kit_helper);
    logu_("refresh_kit: home=%d (kit:%d), away=%d (kit:%d)\n",
        decode_team_id(_ksi->task_uniform_impl->home_team_id_encoded),
        _ksi->task_uniform_impl->home_player_kit_id,
        decode_team_id(_ksi->task_uniform_impl->away_team_id_encoded),
        _ksi->task_uniform_impl->away_player_kit_id
    );

    // check for ongoing refresh
    if (home_or_away == 0 && _ksi->task_uniform_impl->home_change_flag1 == 1) {
        logu_("refresh_kit:: home refresh already requested. Skipping this one\n");
        return 0;
    }
    if (home_or_away == 1 && _ksi->task_uniform_impl->away_change_flag1 == 1) {
        logu_("refresh_kit:: away refresh already requested. Skipping this one\n");
        return 0;
    }

    // flip change flag
    logu_("refresh_kit:: kis->kit_helper->change_flag was: %d\n", kis->kit_helper->change_flag);
    kis->kit_helper->change_flag = (kis->kit_helper->change_flag == 0) ? 1 : 0;

    // flip TaskIniformImpl change flag
    if (home_or_away == 0) {
        _ksi->task_uniform_impl->home_change_flag1 = 1;
    }
    else {
        _ksi->task_uniform_impl->away_change_flag1 = 1;
    }

    lua_pop(L, lua_gettop(L));
    return 0;
}

static int sider_input_is_blocked(lua_State *L)
{
    lua_pushboolean(L, _block_input);
    lua_pushboolean(L, _hard_block);
    return 2;
}

static int sider_input_set_blocked(lua_State *L)
{
    int val = 0;
    if (lua_gettop(L) > 0) {
        val = lua_toboolean(L, 1);
        lua_pop(L, 1);
    }
    int hard = 0;
    if (lua_gettop(L) > 0) {
        hard = lua_toboolean(L, 1);
        lua_pop(L, 1);
    }
    module_t *m = (module_t*)lua_topointer(L, lua_upvalueindex(1));
    if (!m) {
        lua_pushstring(L, "fatal problem: current module is unknown");
        return lua_error(L);
    }
    module_t *om = (_curr_overlay_m == _modules.end()) ? NULL : (*_curr_overlay_m);
    if (!om || m != om) {
        lua_pushfstring(L, "only module that currently controls overlay can block input");
        return lua_error(L);
    }
    _block_input = (val != 0);
    _hard_block = (hard != 0) && _block_input;
    return 0;
}

static int sider_context_register(lua_State *L)
{
    const char *event_key = luaL_checkstring(L, 1);
    if (!lua_isfunction(L, 2)) {
        lua_pushstring(L, "second argument must be a function");
        return lua_error(L);
    }
    else if (strcmp(event_key, "trophy_rewrite")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_trophy_check = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    else if (strcmp(event_key, "livecpk_make_key")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_lcpk_make_key = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    else if (strcmp(event_key, "livecpk_get_filepath")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_lcpk_get_filepath = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    else if (strcmp(event_key, "livecpk_rewrite")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_lcpk_rewrite = lua_gettop(_curr_m->L);
        _rewrite_count++;
        logu_("Registered for \"%s\" event\n", event_key);
    }
    else if (strcmp(event_key, "livecpk_read")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_lcpk_read = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    else if (strcmp(event_key, "livecpk_data_ready")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_lcpk_data_ready = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    else if (strcmp(event_key, "set_teams")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_set_teams = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    else if (strcmp(event_key, "set_kits")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_set_kits = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    else if (strcmp(event_key, "set_home_team_for_kits")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_set_home_team_for_kits = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    else if (strcmp(event_key, "set_away_team_for_kits")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_set_away_team_for_kits = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
      /*
    else if (strcmp(event_key, "set_tournament_id")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_set_tid = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    */
    else if (strcmp(event_key, "set_match_time")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_set_match_time = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    else if (strcmp(event_key, "set_stadium_choice")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_set_stadium_choice = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    else if (strcmp(event_key, "set_stadium")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_set_stadium = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    else if (strcmp(event_key, "set_conditions")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_set_conditions = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    else if (strcmp(event_key, "set_match_settings")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_set_match_settings = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    else if (strcmp(event_key, "after_set_conditions")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_after_set_conditions = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    else if (strcmp(event_key, "context_reset")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_context_reset = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    else if (strcmp(event_key, "custom_event")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_custom = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    else if (strcmp(event_key, "display_frame")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_display_frame = lua_gettop(_curr_m->L);
        _has_on_frame = true;
        logu_("Registered for \"%s\" event\n", event_key);
    }
    /*
    else if (strcmp(event_key, "set_stadium_for_replay")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_set_stadium_for_replay = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    else if (strcmp(event_key, "set_conditions_for_replay")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_set_conditions_for_replay = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    else if (strcmp(event_key, "after_set_conditions_for_replay")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_after_set_conditions_for_replay = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    */
    else if (strcmp(event_key, "get_ball_name")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_get_ball_name = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    else if (strcmp(event_key, "show")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_show = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    else if (strcmp(event_key, "hide")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_hide = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    else if (strcmp(event_key, "overlay_on")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_overlay_on = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    else if (strcmp(event_key, "key_down")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_key_down = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    else if (strcmp(event_key, "key_up")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_key_up = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    else if (strcmp(event_key, "gamepad_input")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_gamepad_input = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    else if (strcmp(event_key, "get_stadium_name")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_get_stadium_name = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    /*
    else if (strcmp(event_key, "enter_edit_mode")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_enter_edit_mode = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    else if (strcmp(event_key, "exit_edit_mode")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_exit_edit_mode = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    else if (strcmp(event_key, "enter_replay_gallery")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_enter_replay_gallery = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    else if (strcmp(event_key, "exit_replay_gallery")==0) {
        lua_pushvalue(L, -1);
        lua_xmove(L, _curr_m->L, 1);
        _curr_m->evt_exit_replay_gallery = lua_gettop(_curr_m->L);
        logu_("Registered for \"%s\" event\n", event_key);
    }
    */
    else {
        logu_("WARN: trying to register for unknown event: \"%s\"\n",
            event_key);
    }
    lua_pop(L, 2);
    return 0;
}


static void push_context_table(lua_State *L)
{
    lua_newtable(L);

    char *sdir = (char*)Utf8::unicodeToUtf8(sider_dir);
    lua_pushstring(L, sdir);
    Utf8::free(sdir);
    lua_setfield(L, -2, "sider_dir");

    lua_pushcfunction(L, sider_context_register);
    lua_setfield(L, -2, "register");

    lua_pushlightuserdata(L, (void*)sider_custom_event_rbx_hk);
    lua_setfield(L, -2, "custom_evt_rbx");

    // ctx.kits
    lua_newtable(L);
    lua_pushcfunction(L, sider_context_get_current_team_id);
    lua_setfield(L, -2, "get_current_team");
    lua_pushcfunction(L, sider_context_get_current_kit_id);
    lua_setfield(L, -2, "get_current_kit_id");
    lua_pushcfunction(L, sider_context_get_kit);
    lua_setfield(L, -2, "get");
    lua_pushcfunction(L, sider_context_get_gk_kit);
    lua_setfield(L, -2, "get_gk");
    lua_pushcfunction(L, sider_context_set_current_kit_id);
    lua_setfield(L, -2, "set_current_kit_id");
    lua_pushcfunction(L, sider_context_set_kit);
    lua_setfield(L, -2, "set");
    lua_pushcfunction(L, sider_context_set_gk_kit);
    lua_setfield(L, -2, "set_gk");
    //lua_pushcfunction(L, sider_context_set_shirt_colors);
    //lua_setfield(L, -2, "set_shirt_colors");
    //lua_pushcfunction(L, sider_context_get_shirt_colors);
    //lua_setfield(L, -2, "get_shirt_colors");
    lua_pushcfunction(L, sider_context_refresh_kit);
    lua_setfield(L, -2, "refresh");
    lua_setfield(L, -2, "kits");
}

static void push_env_table(lua_State *L, module_t *m)
{
    char *sandbox[] = {
        "assert", "table", "pairs", "ipairs",
        "string", "math", "tonumber", "tostring",
        "unpack", "error", "_VERSION", "type", "io",
        "collectgarbage", "jit",
    };

    lua_newtable(L);
    for (int i=0; i<sizeof(sandbox)/sizeof(char*); i++) {
        lua_pushstring(L, sandbox[i]);
        lua_getglobal(L, sandbox[i]);
        lua_settable(L, -3);
    }
    /* DISABLING FOR NOW, as this is a SECURITY issue
    // extra globals
    for (vector<wstring>::iterator i = _config->_lua_extra_globals.begin();
            i != _config->_lua_extra_globals.end();
            i++) {
        char *name = (char*)Utf8::unicodeToUtf8(i->c_str());
        lua_pushstring(L, name);
        lua_getglobal(L, name);
        if (lua_isnil(L, -1)) {
            logu_("WARNING: Unknown Lua global: %s. Skipping it\n",
                name);
            lua_pop(L, 2);
        }
        else {
            lua_settable(L, -3);
        }
        Utf8::free(name);
    }
    */

    // stripped-down os library: with only time, clock, and date
    char *os_names[] = { "time", "clock", "date" };
    lua_newtable(L);
    lua_getglobal(L, "os");
    for (int i=0; i<sizeof(os_names)/sizeof(char*); i++) {
        lua_getfield(L, -1, os_names[i]);
        lua_setfield(L, -3, os_names[i]);
    }
    lua_pop(L, 1);
    lua_setfield(L, -2, "os");

    lua_pushstring(L, "log");
    lua_pushvalue(L, -2);  // upvalue for sider_log C-function
    lua_pushcclosure(L, sider_log, 1);
    lua_settable(L, -3);
    lua_pushstring(L, "_FILE");
    char *sname = (char*)Utf8::unicodeToUtf8(m->filename->c_str());
    lua_pushstring(L, sname);
    Utf8::free(sname);
    lua_settable(L, -3);

    // input table
    lua_newtable(L);
    lua_pushlightuserdata(L, m);
    lua_pushcclosure(L, sider_input_is_blocked, 1);
    lua_setfield(L, -2, "is_blocked");
    lua_pushlightuserdata(L, m);
    lua_pushcclosure(L, sider_input_set_blocked, 1);
    lua_setfield(L, -2, "set_blocked");
    lua_setfield(L, -2, "input");

    // memory lib
    lua_pushvalue(L, _memory_lib_index);
    lua_setfield(L, -2, "memory");

    // fs lib
    lua_pushvalue(L, _fs_lib_index);
    lua_setfield(L, -2, "fs");

    // audio lib
    lua_pushvalue(L, _audio_lib_index);
    lua_setfield(L, -2, "audio");

    // match lib
    lua_pushvalue(L, _match_lib_index);
    lua_setfield(L, -2, "match");

    // z lib
    init_z_lib(L);
    lua_setfield(L, -2, "zlib");

    /*
    // gameplay lib
    init_gameplay_lib(L);

    // gfx lib
    init_gfx_lib(L);
    */

    // set _G
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "_G");

    // turn off JIT, if disabled
    if (!_config->_jit_enabled) {
        lua_getglobal(L, "jit");
        lua_getfield(L, -1, "off");
        if (lua_pcall(L, 0, 0, 0) != 0) {
            const char *err = luaL_checkstring(L, -1);
            logu_("Problem turning off Just-In-Time compiler: %s\n", err);
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    }

    // load some LuaJIT extenstions
    if (_config->_luajit_extensions_enabled) {
        char *ext[] = { "ffi", "bit" };
        for (int i=0; i<sizeof(ext)/sizeof(char*); i++) {
            lua_getglobal(L, "require");
            lua_pushstring(L, ext[i]);
            if (lua_pcall(L, 1, 1, 0) != 0) {
                const char *err = luaL_checkstring(L, -1);
                logu_("Problem loading LuaJIT module (%s): %s\n. "
                      "Skipping it.\n", ext[i], err);
                lua_pop(L, 1);
                continue;
            }
            else {
                lua_setfield(L, -2, ext[i]);
            }
        }
    }
}

static const void *luaL_checkcdata(lua_State *L, int narg)
{
    if (lua_type(L, narg) != 10) {
        luaL_typerror(L, narg, "cdata");
    }
    return lua_topointer(L, narg);
}

static int sider_kmp_search(lua_State *L) {
    size_t pattern_len;
    const char *pattern = luaL_checklstring(L, 1, &pattern_len);
    const char *frm = *(const char**)luaL_checkcdata(L, 2);
    const char *to = *(const char**)luaL_checkcdata(L, 3);
    //logu_("sider_kmp_search: pattern=%p, pattern_len=%d, frm=%p, to=%p\n", pattern, pattern_len, frm, to);
    const char *res = kmp_search(pattern, pattern_len, frm, to);
    //logu_("sider_kmp_search: res=%p\n", res);
    lua_pop(L, lua_gettop(L));
    if (res) {
        lua_pushlightuserdata(L, (void*)res);
    }
    else {
        lua_pushnil(L);
    }
    return 1;
}

void init_lua_support()
{
    if (_config->_lua_enabled) {
        log_(L"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        log_(L"Initializing Lua module system:\n");

        // load and initialize lua modules
        L = luaL_newstate();
        luaL_openlibs(L);

        // prepare context table
        push_context_table(L);

        // memory library
        init_memlib(L);
        _memory_lib_index = lua_gettop(L);

        // memory library
        init_fslib(L);
        _fs_lib_index = lua_gettop(L);

        // audio library
        init_audio_lib(L);
        _audio_lib_index = lua_gettop(L);

        // stats table
        lua_newtable(L);
        _stats_table_index = lua_gettop(L);

        // match table
        lua_newtable(L);
        lua_pushstring(L, "stats");
        lua_pushvalue(L, _stats_table_index);
        lua_pushcclosure(L, sider_match_get_stats, 1);
        lua_settable(L, -3);
        _match_lib_index = lua_gettop(L);

        // kmp_search function
        lua_pushcfunction(L, sider_kmp_search);
        lua_setglobal(L, "sider_kmp_search");

        // jit library
        //luaopen_jit(L);

        // load registered modules
        for (vector<wstring>::iterator it = _config->_module_names.begin();
                it != _config->_module_names.end();
                it++) {
            log_(L"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

            // Use Win32 API to read the script into a buffer:
            // we do not want any nasty surprises with filename encodings
            wstring script_file(sider_dir);
            script_file += L"modules\\";
            script_file += it->c_str();

            log_(L"Loading module: %s ...\n", it->c_str());

            DWORD size = 0;
            HANDLE handle;
            handle = CreateFileW(
                script_file.c_str(),   // file to open
                GENERIC_READ,          // open for reading
                FILE_SHARE_READ,       // share for reading
                NULL,                  // default security
                OPEN_EXISTING,         // existing file only
                FILE_ATTRIBUTE_NORMAL, // normal file
                NULL);                 // no attr. template

            if (handle == INVALID_HANDLE_VALUE)
            {
                log_(L"PROBLEM: Unable to open file: %s\n",
                    script_file.c_str());
                continue;
            }

            FILETIME last_mod_time;
            memset(&last_mod_time, 0, sizeof(FILETIME));
            GetFileTime(handle, NULL, NULL, &last_mod_time);

            size = GetFileSize(handle, NULL);
            BYTE *buf = new BYTE[size+1];
            memset(buf, 0, size+1);
            DWORD bytesRead = 0;
            if (!ReadFile(handle, buf, size, &bytesRead, NULL)) {
                log_(L"PROBLEM: ReadFile error for lua module: %s\n",
                    it->c_str());
                CloseHandle(handle);
                continue;
            }
            CloseHandle(handle);
            // script is now in memory

            char *mfilename = (char*)Utf8::unicodeToUtf8(it->c_str());
            string mfile(mfilename);
            Utf8::free(mfilename);
            int r = luaL_loadbuffer(L, (const char*)buf, size, mfile.c_str());
            delete buf;
            if (r != 0) {
                const char *err = lua_tostring(L, -1);
                logu_("Lua module loading problem: %s. "
                      "Skipping it\n", err);
                lua_pop(L, 1);
                continue;
            }

            module_t *m = new module_t();
            memset(m, 0, sizeof(module_t));
            m->filename = new wstring(it->c_str());
            m->last_modified = last_mod_time;
            //m->cache = new lookup_cache_t(_config->_cache_size);
            m->L = luaL_newstate();

            // set environment
            push_env_table(L, m);
            lua_setfenv(L, -2);

            // run the module
            if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
                const char *err = lua_tostring(L, -1);
                logu_("Lua module initializing problem: %s. "
                      "Skipping it\n", err);
                lua_pop(L, 1);
                continue;
            }

            // check that module chunk is correctly constructed:
            // it must return a table
            if (!lua_istable(L, -1)) {
                logu_("PROBLEM: Lua module (%s) must return a table. "
                      "Skipping it\n", mfile.c_str());
                lua_pop(L, 1);
                continue;
            }

            // now we have module table on the stack
            // run its "init" method, with a context object
            lua_getfield(L, -1, "init");
            if (!lua_isfunction(L, -1)) {
                logu_("PROBLEM: Lua module (%s) does not "
                      "have \"init\" function. Skipping it.\n",
                      mfile.c_str());
                lua_pop(L, 1);
                continue;
            }

            _curr_m = m;

            lua_pushvalue(L, 1); // ctx
            if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
                const char *err = lua_tostring(L, -1);
                logu_("PROBLEM: Lua module (%s) \"init\" function "
                      "returned an error: %s\n", mfile.c_str(), err);
                logu_("Module (%s) is NOT activated\n", mfile.c_str());
                lua_pop(L, 1);
                // pop the module table too, since we are not using it
                lua_pop(L, 1);
            }
            else {
                m->stack_position = lua_gettop(L);
                logu_("OK: Lua module initialized: %s (stack position: %d)\n", mfile.c_str(), m->stack_position);
                //logu_("gettop: %d\n", lua_gettop(L));

                // add to vector of loaded modules
                _modules.push_back(m);
            }
        }
        _curr_overlay_m = _modules.end();
        vector<module_t*>::iterator j;
        for (j = _modules.begin(); j != _modules.end(); j++) {
            module_t *m = *j;
            if (m->evt_overlay_on) {
                _curr_overlay_m = j;
                break;
            }
        }
        log_(L"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        log_(L"Lua module system initialized.\n");
        log_(L"Active modules: %d\n", _modules.size());
        log_(L"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    }
}

void lua_reload_modified_modules()
{
    lock_t lock(&_cs);
    vector<module_t*>::iterator j;
    int count = 0;
    log_(L"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    log_(L"Reloading modified modules ...\n");

    for (j = _modules.begin(); j != _modules.end(); j++) {
        module_t *m = *j;

        wstring script_file(sider_dir);
        script_file += L"modules\\";
        script_file += m->filename->c_str();

        DWORD size = 0;
        HANDLE handle;
        handle = CreateFileW(
            script_file.c_str(),   // file to open
            GENERIC_READ,          // open for reading
            FILE_SHARE_READ,       // share for reading
            NULL,                  // default security
            OPEN_EXISTING,         // existing file only
            FILE_ATTRIBUTE_NORMAL, // normal file
            NULL);                 // no attr. template

        if (handle == INVALID_HANDLE_VALUE)
        {
            log_(L"Reloading skipped because: PROBLEM: Unable to open file: %s\n",
                script_file.c_str());
            continue;
        }

        FILETIME last_mod_time;
        memset(&last_mod_time, 0, sizeof(FILETIME));
        GetFileTime(handle, NULL, NULL, &last_mod_time);

        uint64_t *a = (uint64_t*)&last_mod_time;
        uint64_t *b = (uint64_t*)&(m->last_modified);

        if (!(*a > *b)) {
            // not modified since last load
            CloseHandle(handle);
            continue;
        }

        log_(L"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        log_(L"Modified module: %s ...\n", m->filename->c_str());

        size = GetFileSize(handle, NULL);
        BYTE *buf = new BYTE[size+1];
        memset(buf, 0, size+1);
        DWORD bytesRead = 0;
        if (!ReadFile(handle, buf, size, &bytesRead, NULL)) {
            log_(L"PROBLEM: ReadFile error for lua module: %s\n",
                m->filename->c_str());
            CloseHandle(handle);
            continue;
        }
        CloseHandle(handle);
        // script is now in memory

        char *mfilename = (char*)Utf8::unicodeToUtf8(m->filename->c_str());
        string mfile(mfilename);
        Utf8::free(mfilename);
        int r = luaL_loadbuffer(L, (const char*)buf, size, mfile.c_str());
        delete buf;
        if (r != 0) {
            const char *err = lua_tostring(L, -1);
            logu_("Lua module reloading PROBLEM: %s.\n", err);
            logu_("WARNING: We are keeping the old version in memory\n");
            lua_pop(L, 1);
            continue;
        }

        module_t *newm = new module_t();
        memset(newm, 0, sizeof(module_t));
        newm->filename = new wstring(m->filename->c_str());
        newm->last_modified = last_mod_time;
        //newm->cache = new lookup_cache_t(_config->_cache_size);
        newm->L = luaL_newstate();

        // set environment
        // use the original pointer, because we will copy the module_t structure later
        push_env_table(L, m);
        lua_setfenv(L, -2);

        // run the module
        if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
            const char *err = lua_tostring(L, -1);
            logu_("Lua module initializing problem: %s. "
                  "Reloading cancelled\n", err);
            lua_pop(L, 1);
            continue;
        }

        // check that module chunk is correctly constructed:
        // it must return a table
        if (!lua_istable(L, -1)) {
            logu_("PROBLEM: Lua module (%s) must return a table. "
                  "Reloading cancelled\n", mfile.c_str());
            lua_pop(L, 1);
            continue;
        }

        // now we have module table on the stack
        // run its "init" method, with a context object
        lua_getfield(L, -1, "init");
        if (!lua_isfunction(L, -1)) {
            logu_("PROBLEM: Lua module (%s) does not "
                  "have \"init\" function. Reloading cancelled.\n",
                  mfile.c_str());
            lua_pop(L, 1);
            continue;
        }

        _curr_m = newm;

        lua_pushvalue(L, 1); // ctx
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            const char *err = lua_tostring(L, -1);
            logu_("PROBLEM: Lua module (%s) \"init\" function "
                  "returned an error: %s\n", mfile.c_str(), err);
            logu_("Reloading cancelled\n");
            lua_pop(L, 1);
            // pop the module table too, since we are not using it
            lua_pop(L, 1);

            // clean up
            lua_close(newm->L);
            //delete newm->cache;
            delete newm;
        }
        else {
            newm->stack_position = m->stack_position;
            log_(L"RELOAD OK: Lua module initialized: %s (stack position: %d)\n", newm->filename->c_str(), newm->stack_position);
            memcpy(m, newm, sizeof(module_t));  // new version takes over
            lua_replace(L, newm->stack_position); // move to original module position on the stack
            count++;

            //logu_("gettop: %d\n", lua_gettop(L));

            // cleanup old state
            // todo: figure out a safe way

            // check if need to advance _curr_overlay_m iterator
            if (_curr_overlay_m != _modules.end() && m == *_curr_overlay_m) {
                if (!m->evt_overlay_on) {
                    // this module no longer supports evt_overlay_on
                    // need to switch to another
                    bool switched(false);
                    vector<module_t*>::iterator k = _curr_overlay_m;
                    k++;
                    for (; k != _modules.end(); k++) {
                        module_t *newm = *k;
                        if (newm->evt_overlay_on) {
                            _curr_overlay_m = k;
                            switched = true;
                            break;
                        }
                    }
                    // go again from the start, if not switched yet
                    if (!switched) {
                        vector<module_t*>::iterator k;
                        for (k = _modules.begin(); k != _modules.end(); k++) {
                            module_t *newm = *k;
                            if (newm->evt_overlay_on) {
                                _curr_overlay_m = k;
                                break;
                            }
                        }
                    }
                }
            }
            else if (_curr_overlay_m == _modules.end()) {
                vector<module_t*>::iterator j;
                for (j = _modules.begin(); j != _modules.end(); j++) {
                    module_t *m = *j;
                    if (m->evt_overlay_on) {
                        _curr_overlay_m = j;
                        break;
                    }
                }
            }
            // reload finished
        }
    }
    log_(L"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    log_(L"Reloaded modules: %d\n", count);
    log_(L"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}

void _install_func(IMAGE_SECTION_HEADER *h, int npatt, BYTE **frag, size_t *frag_len, int *offs, BYTE ***addrs, hook_cache_t &hcache);
bool hook_if_all_found();
bool all_found(config_t *cfg);

DWORD install_func(LPVOID thread_param) {
    log_(L"DLL attaching to (%s).\n", module_filename);
    log_(L"Mapped into PES.\n");
    log_(L"sider dir: %s\n", sider_dir);
    logu_("UTF-8 check: ленинградское время ноль часов ноль минут.\n");

    _is_game = true;
    _is_edit_mode = false;

    // initialize filename replacement trick
    strncpy(_file_to_lookup, KNOWN_FILENAME, sizeof(_file_to_lookup)-1);
    *(DWORD*)(_file_to_lookup + strlen(_file_to_lookup) + 1) = MAGIC;
    _file_to_lookup_size = strlen(_file_to_lookup) + 1 + 4 + 1;

    InitializeCriticalSection(&_cs);
    //_key_cache = new cache_t(_config->_key_cache_ttl_sec);
    _small_key_cache = new cache2_t(_config->_cache_size, _config->_key_cache_ttl_sec);

    //_rewrite_cache = new cache2_t(_config->_cache_size, _config->_rewrite_cache_ttl_sec);
#ifdef PERF_TESTING
    _stats = new stats_t(L"have_live");
    _content_stats = new stats_t(L"have_content");
    _fileops_stats = new stats_t(L"fileops");
    _overlay_stats = new stats_t(L"overlay_on");
#endif

    //_lookup_cache = new lookup_cache_t(_config->_cache_size);

    InitializeCriticalSection(&_tcs);
    _trophy_table_copy_count = 0;
    memset(_trophy_map, 0, sizeof(_trophy_map));

    log_(L"debug = %d\n", _config->_debug);
    //if (_config->_game_speed) {
    //    log_(L"game.speed = %0.3f\n", *(_config->_game_speed));
    //}
    log_(L"skip.checks = %d\n", _config->_skip_checks);
    log_(L"game.priority.class = 0x%x\n", _config->_priority_class);
    log_(L"livecpk.enabled = %d\n", _config->_livecpk_enabled);
    log_(L"lookup-cache.enabled = %d\n", _config->_lookup_cache_enabled);
    log_(L"lua.enabled = %d\n", _config->_lua_enabled);
    log_(L"lua.gc.opt = %s\n", (_config->_lua_gc_opt == LUA_GCSTEP)? L"step" : L"collect");
    log_(L"jit.enabled = %d\n", _config->_jit_enabled);
    log_(L"luajit.ext.enabled = %d\n", _config->_luajit_extensions_enabled);
    log_(L"dummify.uniparam = %d\n", _config->_dummify_uniparam);
    //log_(L"address-cache.enabled = %d\n", (int)(!_config->_ac_off));
    log_(L"key-cache.ttl-sec = %d\n", _config->_key_cache_ttl_sec);
    log_(L"rewrite-cache.ttl-sec = %d\n", _config->_rewrite_cache_ttl_sec);
    log_(L"cache.size = %d\n", _config->_cache_size);
    log_(L"start.minimized = %d\n", _config->_start_minimized);
    log_(L"free.side.select = %d\n", _config->_free_side_select);
    log_(L"overlay.enabled = %d\n", _config->_overlay_enabled);
    log_(L"overlay.on-from-start = %d\n", _config->_overlay_on_from_start);
    log_(L"overlay.font = %s\n", _config->_overlay_font.c_str());
    log_(L"overlay.text-color = 0x%08x\n", _config->_overlay_text_color);
    log_(L"overlay.background-color = 0x%08x\n", _config->_overlay_background_color);
    log_(L"overlay.image-alpha-max = %0.3f\n", _config->_overlay_image_alpha_max);
    log_(L"overlay.location = %d\n", _config->_overlay_location);
    log_(L"overlay.font-size = %d\n", _config->_overlay_font_size);
    log_(L"overlay.vkey.toggle = 0x%02x\n", _config->_overlay_vkey_toggle);
    log_(L"overlay.vkey.next-module = 0x%02x\n", _config->_overlay_vkey_next_module);
    log_(L"overlay.vkey.prev-module = 0x%02x\n", _config->_overlay_vkey_prev_module);
    log_(L"overlay.toggle.sound = %s\n", _config->_overlay_toggle_sound.c_str());
    log_(L"overlay.toggle.sound-volume = %0.2f\n", _config->_overlay_toggle_sound_volume);
    log_(L"overlay.block-input-when-on = %d\n", _config->_global_block_input);
    log_(L"overlay.gamepad-input-blocking.enabled = %d\n", _config->_controller_input_blocking_enabled);
    log_(L"match-stats.enabled = %d\n", _config->_match_stats_enabled);
    log_(L"vkey.reload-1 = 0x%02x\n", _config->_vkey_reload_1);
    log_(L"vkey.reload-2 = 0x%02x\n", _config->_vkey_reload_2);
    log_(L"close.on.exit = %d\n", _config->_close_sider_on_exit);
    log_(L"start.game = %s\n", _config->_start_game.c_str());
    log_(L"match.minutes = %d\n", _config->_num_minutes);

    log_(L"--------------------------\n");
    log_(L"gamepad.dinput.enabled = %d\n", _gamepad_config->_dinput_enabled);
    log_(L"gamepad.xinput.enabled = %d\n", _gamepad_config->_xinput_enabled);
    log_(L"gamepad.poll-interval-msec = %d\n", _gamepad_config->_gamepad_poll_interval_msec);
    log_(L"gamepad.overlay.poll-interval-msec = %d\n", _gamepad_config->_gamepad_overlay_poll_interval_msec);
    log_(L"gamepad.stick-sensitivity = %0.3f\n", _gamepad_config->_stick_sensitivity);
    log_(L"gamepad.overlay.toggle-1 = %s\n", _xi_names[_gamepad_config->_overlay_toggle_1]);
    log_(L"gamepad.overlay.toggle-2 = %s\n", _xi_names[_gamepad_config->_overlay_toggle_2]);
    log_(L"gamepad.overlay.next-module = %s\n", _xi_names[_gamepad_config->_overlay_next_module]);
    log_(L"gamepad.overlay.prev-module = %s\n", _xi_names[_gamepad_config->_overlay_prev_module]);
    if (_gamepad_config->_dinput_enabled) {
        log_(L"--------------------------\n");
        log_(L"DirectInput controls map: %d items\n", _gamepad_config->_di_map.size());
        for (unordered_map<DWORD,gamepad_directinput_mapping_t>::iterator it = _gamepad_config->_di_map.begin();
                it != _gamepad_config->_di_map.end();
                it++) {
            log_(L"directinput.map: 0x%x --> %s\n", it->first, _xi_names[it->second.what]);
        }
    }

    log_(L"--------------------------\n");
    log_(L"Gamepad-to-keyboard mapping: %d items\n", _gamepad_config->_vkey_map.size());
    for (unordered_map<uint64_t,gamepad_vkey_mapping_t>::iterator it = _gamepad_config->_vkey_map.begin();
            it != _gamepad_config->_vkey_map.end();
            it++) {
        log_(L"gamepad.keyboard.mapping: 0x%016llx --> (%s,%d,0x%x)\n", it->first,
            _xi_names[it->second.what], it->second.value, it->second.vkey);
    }

    log_(L"--------------------------\n");
    log_(L"hook.set-team-id = %d\n", _config->_hook_set_team_id);
    log_(L"hook.set-settings = %d\n", _config->_hook_set_settings);
    log_(L"hook.context-reset = %d\n", _config->_hook_context_reset);
    log_(L"hook.trophy-table = %d\n", _config->_hook_trophy_table);
    log_(L"hook.trophy-check = %d\n", _config->_hook_trophy_check);
    log_(L"hook.all-keyboards = %d\n", _config->_hook_all_keyboards);
    log_(L"--------------------------\n");

    for (vector<wstring>::iterator it = _config->_cpk_roots.begin();
            it != _config->_cpk_roots.end();
            it++) {
        log_(L"Using cpk.root: %s\n", it->c_str());
    }

    // read hook cache
    wstring cache_file(sider_dir);
    cache_file += L"startup.cache";
    hook_cache_t hcache(cache_file);

    // prepare patterns
#define NUM_PATTERNS 41
    BYTE *frag[NUM_PATTERNS+1];
    frag[1] = lcpk_pattern_at_read_file;
    frag[2] = lcpk_pattern_at_get_size;
    frag[3] = lcpk_pattern_at_write_cpk_filesize;
    frag[4] = lcpk_pattern_at_mem_copy;
    frag[5] = lcpk_pattern_at_lookup_file;
    frag[6] = pattern_set_team_id;
    frag[7] = pattern_set_settings;
    frag[8] = pattern_trophy_check;
    frag[9] = pattern_context_reset;
    frag[10] = pattern_set_min_time;
    frag[11] = pattern_set_max_time;
    frag[12] = pattern_call_set_minutes;
    frag[13] = pattern_sider;
    frag[14] = pattern_trophy_table;
    frag[15] = pattern_ball_name;
    frag[16] = pattern_dxgi;
    frag[17] = pattern_set_stadium_choice;
    frag[18] = pattern_stadium_name;
    frag[19] = pattern_def_stadium_name;
    frag[20] = pattern2_set_settings;
    frag[21] = pattern_check_kit_choice;
    frag[22] = pattern_get_uniparam;
    frag[23] = pattern_data_ready;
    frag[24] = lcpk_pattern2_at_read_file;
    frag[25] = pattern_kit_status;
    frag[26] = pattern_set_team_for_kits;
    frag[27] = pattern_clear_team_for_kits;
    frag[28] = pattern_uniparam_loaded;
    frag[29] = pattern_call_to_move;
    frag[30] = lcpk_pattern2_at_mem_copy;
    frag[31] = lcpk_pattern2_at_lookup_file;
    frag[32] = lcpk_pattern2_at_write_cpk_filesize;
    frag[33] = lcpk_pattern3_at_read_file;
    frag[34] = pattern2_set_stadium_choice;
    frag[35] = pattern2_call_to_move;
    frag[36] = pattern_copy_clock;
    frag[37] = pattern_clear_sc;
    frag[38] = pattern_xinput;
    frag[39] = pattern_game_lite;
    frag[40] = pattern_trophy_check2;
    frag[41] = pattern_set_edit_team_id;

    memset(_variations, 0xff, sizeof(_variations));
    _variations[1] = 24;
    _variations[3] = 32;
    _variations[4] = 30;
    _variations[5] = 31;
    _variations[7] = 20;
    _variations[8] = 40;
    _variations[15] = 39;
    _variations[17] = 34;
    _variations[20] = 7;
    _variations[24] = 1;
    _variations[29] = 35;
    _variations[30] = 4;
    _variations[31] = 5;
    _variations[32] = 3;
    _variations[33] = 1;
    _variations[34] = 17;
    _variations[35] = 29;
    _variations[39] = 15;
    _variations[40] = 8;

    size_t frag_len[NUM_PATTERNS+1];
    frag_len[1] = _config->_livecpk_enabled ? sizeof(lcpk_pattern_at_read_file)-1 : 0;
    frag_len[2] = _config->_livecpk_enabled ? sizeof(lcpk_pattern_at_get_size)-1 : 0;
    frag_len[3] = _config->_livecpk_enabled ? sizeof(lcpk_pattern_at_write_cpk_filesize)-1 : 0;
    frag_len[4] = _config->_livecpk_enabled ? sizeof(lcpk_pattern_at_mem_copy)-1 : 0;
    frag_len[5] = _config->_livecpk_enabled ? sizeof(lcpk_pattern_at_lookup_file)-1 : 0;
    frag_len[6] = _config->_lua_enabled ? sizeof(pattern_set_team_id)-1 : 0;
    frag_len[7] = _config->_lua_enabled ? sizeof(pattern_set_settings)-1 : 0;
    frag_len[8] = _config->_lua_enabled ? sizeof(pattern_trophy_check)-1 : 0;
    frag_len[9] = _config->_lua_enabled ? sizeof(pattern_context_reset)-1 : 0;
    frag_len[10] = 0; //sizeof(pattern_set_min_time)-1;
    frag_len[11] = sizeof(pattern_set_max_time)-1;
    frag_len[12] = (_config->_num_minutes > 0) ? sizeof(pattern_call_set_minutes)-1 : 0;
    frag_len[13] = _config->_free_side_select ? sizeof(pattern_sider)-1 : 0;
    frag_len[14] = _config->_lua_enabled ? sizeof(pattern_trophy_table)-1 : 0;
    frag_len[15] = _config->_lua_enabled ? sizeof(pattern_ball_name)-1 : 0;
    frag_len[16] = _config->_overlay_enabled ? sizeof(pattern_dxgi)-1 : 0;
    frag_len[17] = _config->_lua_enabled ? sizeof(pattern_set_stadium_choice)-1 : 0;
    frag_len[18] = _config->_lua_enabled ? sizeof(pattern_stadium_name)-1 : 0;
    frag_len[19] = _config->_lua_enabled ? sizeof(pattern_def_stadium_name)-1 : 0;
    frag_len[20] = _config->_lua_enabled ? sizeof(pattern2_set_settings)-1 : 0;
    frag_len[21] = _config->_lua_enabled ? sizeof(pattern_check_kit_choice)-1 : 0;
    frag_len[22] = _config->_lua_enabled ? sizeof(pattern_get_uniparam)-1 : 0;
    frag_len[23] = _config->_lua_enabled ? sizeof(pattern_data_ready)-1 : 0;
    frag_len[24] = _config->_livecpk_enabled ? sizeof(lcpk_pattern2_at_read_file)-1 : 0;
    frag_len[25] = _config->_lua_enabled ? sizeof(pattern_kit_status)-1 : 0;
    frag_len[26] = _config->_lua_enabled ? sizeof(pattern_set_team_for_kits)-1 : 0;
    frag_len[27] = _config->_lua_enabled ? sizeof(pattern_clear_team_for_kits)-1 : 0;
    frag_len[28] = _config->_lua_enabled ? sizeof(pattern_uniparam_loaded)-1 : 0;
    frag_len[29] = _config->_lua_enabled ? sizeof(pattern_call_to_move)-1 : 0;
    frag_len[30] = _config->_livecpk_enabled ? sizeof(lcpk_pattern2_at_mem_copy)-1 : 0;
    frag_len[31] = _config->_livecpk_enabled ? sizeof(lcpk_pattern2_at_lookup_file)-1 : 0;
    frag_len[32] = _config->_livecpk_enabled ? sizeof(lcpk_pattern2_at_write_cpk_filesize)-1 : 0;
    frag_len[33] = _config->_livecpk_enabled ? sizeof(lcpk_pattern3_at_read_file)-1 : 0;
    frag_len[34] = _config->_lua_enabled ? sizeof(pattern2_set_stadium_choice)-1 : 0;
    frag_len[35] = _config->_lua_enabled ? sizeof(pattern2_call_to_move)-1 : 0;
    frag_len[36] = _config->_lua_enabled ? sizeof(pattern_copy_clock)-1 : 0;
    frag_len[37] = _config->_lua_enabled ? sizeof(pattern_clear_sc)-1 : 0;
    frag_len[38] = _config->_lua_enabled ? sizeof(pattern_xinput)-1 : 0;
    frag_len[39] = _config->_lua_enabled ? sizeof(pattern_game_lite)-1 : 0;
    frag_len[40] = _config->_lua_enabled ? sizeof(pattern_trophy_check2)-1 : 0;
    frag_len[41] = _config->_lua_enabled ? sizeof(pattern_set_edit_team_id)-1 : 0;

    int offs[NUM_PATTERNS+1];
    offs[1] = lcpk_offs_at_read_file;
    offs[2] = lcpk_offs_at_get_size;
    offs[3] = lcpk_offs_at_write_cpk_filesize;
    offs[4] = lcpk_offs_at_mem_copy;
    offs[5] = lcpk_offs_at_lookup_file;
    offs[6] = offs_set_team_id;
    offs[7] = offs_set_settings;
    offs[8] = offs_trophy_check;
    offs[9] = offs_context_reset;
    offs[10] = offs_set_min_time;
    offs[11] = offs_set_max_time;
    offs[12] = offs_call_set_minutes;
    offs[13] = offs_sider;
    offs[14] = offs_trophy_table;
    offs[15] = offs_ball_name;
    offs[16] = offs_dxgi;
    offs[17] = offs_set_stadium_choice;
    offs[18] = offs_stadium_name;
    offs[19] = offs_def_stadium_name;
    offs[20] = offs_set_settings;
    offs[21] = offs_check_kit_choice;
    offs[22] = offs_get_uniparam;
    offs[23] = offs_data_ready;
    offs[24] = lcpk_offs2_at_read_file;
    offs[25] = offs_kit_status;
    offs[26] = offs_set_team_for_kits;
    offs[27] = offs_clear_team_for_kits;
    offs[28] = offs_uniparam_loaded;
    offs[29] = offs_call_to_move;
    offs[30] = lcpk_offs_at_mem_copy;
    offs[31] = lcpk_offs_at_lookup_file;
    offs[32] = lcpk_offs_at_write_cpk_filesize;
    offs[33] = lcpk_offs3_at_read_file;
    offs[34] = offs2_set_stadium_choice;
    offs[35] = offs2_call_to_move;
    offs[36] = offs_copy_clock;
    offs[37] = offs_clear_sc;
    offs[38] = offs_xinput;
    offs[39] = offs_game_lite;
    offs[40] = offs_trophy_check2;
    offs[41] = offs_set_edit_team_id;

    BYTE **addrs[NUM_PATTERNS+1];
    addrs[1] = &_config->_hp_at_read_file;
    addrs[2] = &_config->_hp_at_get_size;
    addrs[3] = &_config->_hp_at_extend_cpk;
    addrs[4] = &_config->_hp_at_mem_copy;
    addrs[5] = &_config->_hp_at_lookup_file;
    addrs[6] = &_config->_hp_at_set_team_id;
    addrs[7] = &_config->_hp_at_set_settings;
    addrs[8] = &_config->_hp_at_trophy_check;
    addrs[9] = &_config->_hp_at_context_reset;
    addrs[10] = &_config->_hp_at_set_min_time;
    addrs[11] = &_config->_hp_at_set_max_time;
    addrs[12] = &_config->_hp_at_call_set_minutes;
    addrs[13] = &_config->_hp_at_sider;
    addrs[14] = &_config->_hp_at_trophy_table;
    addrs[15] = &_config->_hp_at_ball_name;
    addrs[16] = &_config->_hp_at_dxgi;
    addrs[17] = &_config->_hp_at_set_stadium_choice;
    addrs[18] = &_config->_hp_at_stadium_name;
    addrs[19] = &_config->_hp_at_def_stadium_name;
    addrs[20] = &_config->_hp_at_set_settings;
    addrs[21] = &_config->_hp_at_check_kit_choice;
    addrs[22] = &_config->_hp_at_get_uniparam;
    addrs[23] = &_config->_hp_at_data_ready;
    addrs[24] = &_config->_hp_at_read_file;
    addrs[25] = &_config->_hp_at_kit_status;
    addrs[26] = &_config->_hp_at_set_team_for_kits;
    addrs[27] = &_config->_hp_at_clear_team_for_kits;
    addrs[28] = &_config->_hp_at_uniparam_loaded;
    addrs[29] = &_config->_hp_at_call_to_move;
    addrs[30] = &_config->_hp_at_mem_copy;
    addrs[31] = &_config->_hp_at_lookup_file;
    addrs[32] = &_config->_hp_at_extend_cpk;
    addrs[33] = &_config->_hp_at_read_file;
    addrs[34] = &_config->_hp_at_set_stadium_choice;
    addrs[35] = &_config->_hp_at_call_to_move;
    addrs[36] = &_config->_hp_at_copy_clock;
    addrs[37] = &_config->_hp_at_clear_sc;
    addrs[38] = &_config->_hp_at_xinput;
    addrs[39] = &_config->_hp_at_game_lite;
    addrs[40] = &_config->_hp_at_trophy_check2;
    addrs[41] = &_config->_hp_at_set_edit_team_id;

    // check hook cache first
    for (int i=0;; i++) {
        IMAGE_SECTION_HEADER *h = GetSectionHeaderByOrdinal(i);
        if (!h) {
            break;
        }

        BYTE* base = (BYTE*)GetModuleHandle(NULL);
        base += h->VirtualAddress;

        char name[16];
        memset(name, 0, sizeof(name));
        memcpy(name, h->Name, 8);
        logu_("Examining code section: %s\n", name);
        if (h->Misc.VirtualSize < 0x10) {
            log_(L"Section too small: %s (%p). Skipping\n", name, h->Misc.VirtualSize);
            continue;
        }

        for (int j=1; j<NUM_PATTERNS+1; j++) {
            if (frag_len[j]==0) {
                // empty frag
                continue;
            }
            if (*(addrs[j])) {
                // already found
                continue;
            }
            void *hint = hcache.get(j);
            if (hint) {
                BYTE *p = check_hint(base, h->Misc.VirtualSize,
                    frag[j], frag_len[j], hint);
                if (p) {
                    if (_variations[j]!=0xff) {
                        log_(L"Found pattern (hint match) %i (%i", j, _variations[j]);
                        for (int k=0; k<sizeof(_variations)/sizeof(BYTE); k++) {
                            if (_variations[j] != k && _variations[k] == j) {
                                log_(L",%i", k);
                            }
                        }
                        log_(L") of %i (at %p)\n", NUM_PATTERNS, p);
                    }
                    else {
                        log_(L"Found pattern (hint match) %i of %i (at %p)\n", j, NUM_PATTERNS, p);
                    }
                    *(addrs[j]) = p + offs[j];
                }
            }
        }
        if (all_found(_config)) {
            break;
        }
    }

    if (!all_found(_config)) for (int i=0;; i++) {
        IMAGE_SECTION_HEADER *h = GetSectionHeaderByOrdinal(i);
        if (!h) {
            break;
        }

        char name[16];
        memset(name, 0, sizeof(name));
        memcpy(name, h->Name, 8);
        logu_("Examining code section: %s\n", name);
        if (h->Misc.VirtualSize < 0x10) {
            log_(L"Section too small: %s (%p). Skipping\n", name, h->Misc.VirtualSize);
            continue;
        }

        _install_func(h, NUM_PATTERNS, frag, frag_len, offs, addrs, hcache);
        if (all_found(_config)) {
            break;
        }
    }

    if (hook_if_all_found()) {
        hcache.save();
        memset(&_prev_xi_state, 0, sizeof(xi_state_t));
        memset(_xi_changes, 0, sizeof(_xi_changes));
        init_fast_pov_map();
        init_direct_input();
        init_lua_support();
    }

    log_(L"Sider initialization complete.\n");
    return 0;
}

bool all_found(config_t *cfg) {
    bool all(true);
    if (cfg->_livecpk_enabled) {
        all = all && (
            cfg->_hp_at_read_file > 0 &&
            cfg->_hp_at_get_size > 0 &&
            cfg->_hp_at_extend_cpk > 0 &&
            cfg->_hp_at_mem_copy > 0 &&
            cfg->_hp_at_lookup_file > 0
        );
    }
    if (cfg->_lua_enabled) {
        //all = all && true;
        all = all && (
            cfg->_hp_at_set_team_id > 0 &&
            cfg->_hp_at_set_settings > 0 &&
            (cfg->_hp_at_trophy_check > 0 || cfg->_hp_at_trophy_check2 > 0) &&
            cfg->_hp_at_trophy_table > 0 &&
            (cfg->_hp_at_ball_name > 0 || cfg->_hp_at_game_lite > 0) &&
            cfg->_hp_at_stadium_name > 0 &&
            cfg->_hp_at_def_stadium_name > 0 &&
            cfg->_hp_at_context_reset > 0 &&
            cfg->_hp_at_set_stadium_choice > 0 &&
            cfg->_hp_at_check_kit_choice > 0 &&
            cfg->_hp_at_get_uniparam > 0 &&
            cfg->_hp_at_data_ready > 0 &&
            cfg->_hp_at_call_to_move > 0 &&
            cfg->_hp_at_kit_status > 0 &&
            cfg->_hp_at_set_team_for_kits > 0 &&
            cfg->_hp_at_clear_team_for_kits > 0 &&
            cfg->_hp_at_uniparam_loaded > 0 &&
            cfg->_hp_at_copy_clock > 0 &&
            cfg->_hp_at_clear_sc > 0 &&
            cfg->_hp_at_xinput > 0 &&
            (cfg->_hp_at_set_edit_team_id > 0 || cfg->_hp_at_game_lite > 0) &&
            true
        );
    }
    if (cfg->_num_minutes > 0) {
        all = all && (
            //cfg->_hp_at_set_min_time > 0 &&
            cfg->_hp_at_set_max_time > 0 &&
            cfg->_hp_at_call_set_minutes > 0
        );
    }
    if (cfg->_overlay_enabled) {
        all = all && (
            cfg->_hp_at_dxgi > 0
        );
    }
    if (cfg->_free_side_select) {
        all = all && (
            cfg->_hp_at_sider > 0
        );
    }
    return all;
}

void _install_func(IMAGE_SECTION_HEADER *h, int npatt, BYTE **frag, size_t *frag_len, int *offs, BYTE ***addrs, hook_cache_t &hcache) {
    BYTE* base = (BYTE*)GetModuleHandle(NULL);
    base += h->VirtualAddress;
    log_(L"Searching range: %p : %p (size: %p)\n",
        base, base + h->Misc.VirtualSize, h->Misc.VirtualSize);

    for (int j=1; j<npatt+1; j++) {
        if (frag_len[j]==0) {
            // empty frag
            continue;
        }
        if (*(addrs[j])) {
            // already found
            continue;
        }
        BYTE *p = find_code_frag(base, h->Misc.VirtualSize,
            frag[j], frag_len[j]);
        if (!p) {
            continue;
        }
        if (_variations[j]!=0xff) {
            log_(L"Found pattern %i (%i", j, _variations[j]);
            for (int k=0; k<sizeof(_variations)/sizeof(BYTE); k++) {
                if (_variations[j] != k && _variations[k] == j) {
                    log_(L",%i", k);
                }
            }
            log_(L") of %i (at %p)\n", NUM_PATTERNS, p);
        }
        else {
            log_(L"Found pattern %i of %i (at %p)\n", j, NUM_PATTERNS, p);
        }
        *(addrs[j]) = p + offs[j];
        hcache.set(j,p);
        if (_variations[j]!=0xff) {
            for (int k=0; k<sizeof(_variations)/sizeof(BYTE); k++) {
                if (_variations[j] != k && _variations[k] == j) {
                    hcache.set(k,0);
                }
            }
        }
    }
}

bool hook_if_all_found() {
    bool result(false);

    if (all_found(_config)) {
        logu_("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        logu_("Success:: all hook points found. Sider is enabled now\n");
        logu_("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        result = true;

        // hooks
        if (_config->_livecpk_enabled) {
            log_(L"sider_read_file_hk: %p\n", sider_read_file_hk);
            log_(L"sider_get_size_hk: %p\n", sider_get_size_hk);
            log_(L"sider_extend_cpk_hk: %p\n", sider_extend_cpk_hk);
            log_(L"sider_mem_copy_hk: %p\n", sider_mem_copy_hk);
            log_(L"sider_lookup_file: %p\n", sider_lookup_file_hk);

            hook_indirect_call(_config->_hp_at_read_file, (BYTE*)sider_read_file_hk);
            hook_call(_config->_hp_at_get_size, (BYTE*)sider_get_size_hk, 0);
            hook_call(_config->_hp_at_extend_cpk, (BYTE*)sider_extend_cpk_hk, 1);
            hook_call(_config->_hp_at_mem_copy, (BYTE*)sider_mem_copy_hk, 0);
            hook_call_rcx(_config->_hp_at_lookup_file, (BYTE*)sider_lookup_file_hk, 3);
        }

        if (_config->_overlay_enabled && _config->_lua_enabled) {
            if (_config->_hp_at_dxgi) {
                BYTE *addr = get_target_addr(_config->_hp_at_dxgi);
                BYTE *loc = get_target_location(addr);
                _org_CreateDXGIFactory1 = *(PFN_CreateDXGIFactory1*)loc;
                logu_("_org_CreateDXGIFactory1: %p\n", _org_CreateDXGIFactory1);
                hook_indirect_call(addr, (BYTE*)sider_CreateDXGIFactory1);
            }
        }

        if (_config->_lua_enabled) {
            log_(L"-------------------------------\n");
            log_(L"sider_set_team_id: %p\n", sider_set_team_id_hk);
            log_(L"sider_set_settings: %p\n", sider_set_settings_hk);
            log_(L"sider_trophy_check: %p\n", sider_trophy_check_hk);
            log_(L"sider_trophy_check2: %p\n", sider_trophy_check2_hk);
            log_(L"sider_trophy_table: %p\n", sider_trophy_table_hk);
            log_(L"sider_context_reset: %p\n", sider_context_reset_hk);
            log_(L"sider_ball_name: %p\n", sider_ball_name_hk);
            log_(L"sider_stadium_name: %p\n", sider_stadium_name_hk);
            log_(L"sider_def_stadium_name: %p\n", sider_def_stadium_name_hk);
            log_(L"sider_set_stadium_choice: %p\n", sider_set_stadium_choice_hk);
            log_(L"sider_check_kit_choice: %p\n", sider_check_kit_choice_hk);
            log_(L"sider_data_ready: %p\n", sider_data_ready_hk);
            log_(L"call_to_move at: %p\n", _config->_hp_at_call_to_move);
            log_(L"sider_custom_event_rbx_hk: %p\n", sider_custom_event_rbx_hk);

            if (_config->_hp_at_set_team_id) {
                BYTE *check_addr = _config->_hp_at_set_team_id - offs_set_team_id + offs_check_set_team_id;
                logu_("_hp_at_set_team_id: %p\n", _config->_hp_at_set_team_id);
                logu_("check_addr: %p\n", check_addr);
                logu_("instruction at check_addr: %02x %02x\n", check_addr[0], check_addr[1]);
                if (memcmp(check_addr, check_set_team_id_1, sizeof(check_set_team_id_1)-1)==0) {
                    logu_("Using 1st variation of set_team_id hook\n");
                    hook_call_rdx_with_head_and_tail(_config->_hp_at_set_team_id, (BYTE*)sider_set_team_id_hk,
                        (BYTE*)pattern_set_team_id_head, sizeof(pattern_set_team_id_head)-1,
                        (BYTE*)pattern_set_team_id_tail_1, sizeof(pattern_set_team_id_tail_1)-1);
                }
                else if (memcmp(check_addr, check_set_team_id_2, sizeof(check_set_team_id_2)-1)==0) {
                    logu_("Using 2nd variation of set_team_id hook\n");
                    hook_call_rdx_with_head_and_tail(_config->_hp_at_set_team_id, (BYTE*)sider_set_team_id_hk,
                        (BYTE*)pattern_set_team_id_head, sizeof(pattern_set_team_id_head)-1,
                        (BYTE*)pattern_set_team_id_tail_2, sizeof(pattern_set_team_id_tail_2)-1);
                }
            }
            if (_config->_hp_at_set_settings)
                hook_call_with_head_and_tail(_config->_hp_at_set_settings, (BYTE*)sider_set_settings_hk,
                    (BYTE*)pattern_set_settings_head, sizeof(pattern_set_settings_head)-1,
                    (BYTE*)pattern_set_settings_tail, sizeof(pattern_set_settings_tail)-1);

            if (_config->_hp_at_trophy_check)
                hook_call_rcx(_config->_hp_at_trophy_check, (BYTE*)sider_trophy_check_hk, 0);
            if (_config->_hp_at_trophy_check2)
                hook_call_rcx(_config->_hp_at_trophy_check2, (BYTE*)sider_trophy_check2_hk, 0);
            if (_config->_hp_at_trophy_table)
                hook_call_rcx(_config->_hp_at_trophy_table, (BYTE*)sider_trophy_table_hk, 0);
            if (_config->_hook_context_reset)
                hook_call(_config->_hp_at_context_reset, (BYTE*)sider_context_reset_hk, 6);

            if (_config->_hp_at_ball_name) {
                hook_call_with_head_and_tail(_config->_hp_at_ball_name, (BYTE*)sider_ball_name_hk,
                    (BYTE*)pattern_ball_name_head, sizeof(pattern_ball_name_head)-1,
                    (BYTE*)pattern_ball_name_tail, sizeof(pattern_ball_name_tail)-1);
            }
            else if (_config->_hp_at_game_lite) {
                // pes 2021 lite: does not work
                logu_("WARN: This looks to be PES Lite. Some sider features will not work\n");
            }

            hook_call_with_head_and_tail(_config->_hp_at_stadium_name, (BYTE*)sider_stadium_name_hk,
                (BYTE*)pattern_stadium_name_head, sizeof(pattern_stadium_name_head)-1,
                (BYTE*)pattern_stadium_name_tail, sizeof(pattern_stadium_name_tail)-1);

            BYTE *call_target = get_target_location2(_config->_hp_at_set_stadium_choice + 1);
            if (call_target) {
                hook_jmp(call_target, (BYTE*)sider_set_stadium_choice_hk, 0);
            }
            //hook_call_with_head_and_tail(_config->_hp_at_set_stadium_choice, (BYTE*)sider_set_stadium_choice_hk,
            //    (BYTE*)pattern_set_stadium_choice_head, sizeof(pattern_set_stadium_choice_head)-1,
            //    (BYTE*)pattern_set_stadium_choice_tail, sizeof(pattern_set_stadium_choice_tail)-1);

            // move next function (right after data_ready), if necessary
            if (*(BYTE*)(_config->_hp_at_data_ready + 11) != 0xcc) {
                // there is no extra 0xcc byte (at offset+12), so we need to move code
                move_code(_config->_hp_at_data_ready + 11, 1, 7);
                DWORD rel_offs = *(DWORD*)(_config->_hp_at_call_to_move + 1) + 1;
                patch_at_location(_config->_hp_at_call_to_move + 1, (BYTE*)&rel_offs, sizeof(DWORD));
            }

            hook_jmp(_config->_hp_at_data_ready, (BYTE*)sider_data_ready_hk, 0);

            hook_call(_config->_hp_at_check_kit_choice, (BYTE*)sider_check_kit_choice_hk, 0);

            _uniparam_base = get_target_location2(_config->_hp_at_get_uniparam);
            log_(L"_uniparam_base = %p\n", _uniparam_base);

            hook_call_rdx(_config->_hp_at_kit_status, (BYTE*)sider_kit_status_hk, 2);

            hook_call_rcx(_config->_hp_at_set_team_for_kits, (BYTE*)sider_set_team_for_kits_hk, 1);
            hook_call(_config->_hp_at_clear_team_for_kits, (BYTE*)sider_clear_team_for_kits_hk, 4);
            hook_call_rdx(_config->_hp_at_uniparam_loaded, (BYTE*)sider_loaded_uniparam_hk, 0);
            if (_config->_match_stats_enabled) {
                hook_call(_config->_hp_at_copy_clock, (BYTE*)sider_copy_clock_hk, 6);
                hook_call(_config->_hp_at_clear_sc, (BYTE*)sider_clear_sc_hk, 2);
            }

            BYTE *old_moved_call = _config->_hp_at_def_stadium_name + def_stadium_name_moved_call_offs_old;
            BYTE *new_moved_call = _config->_hp_at_def_stadium_name + def_stadium_name_moved_call_offs_new;
            hook_call_rdx_with_head_and_tail_and_moved_call(
                _config->_hp_at_def_stadium_name, (BYTE*)sider_def_stadium_name_hk,
                (BYTE*)pattern_def_stadium_name_head, sizeof(pattern_def_stadium_name_head)-1,
                (BYTE*)pattern_def_stadium_name_tail, sizeof(pattern_def_stadium_name_tail)-1,
                old_moved_call, new_moved_call);

            if (_config->_hp_at_set_edit_team_id) {
                hook_call(_config->_hp_at_set_edit_team_id, (BYTE*)sider_set_edit_team_id_hk, 3);
            }

            if (_config->_overlay_enabled && _config->_controller_input_blocking_enabled) {
                HookXInputGetState();
            }

            log_(L"-------------------------------\n");
        }

        if (_config->_free_side_select) {
            log_(L"sider_free_select: %p\n", sider_free_select_hk);
            hook_call_rcx(_config->_hp_at_sider, (BYTE*)sider_free_select_hk, 0);
        }

        //patch_at_location(_config->_hp_at_set_min_time, "\x90\x90\x90\x90\x90\x90\x90", 7);
        patch_at_location(_config->_hp_at_set_max_time, "\x90\x90\x90\x90\x90\x90\x90", 7);

        if (_config->_num_minutes != 0) {

            if (_config->_num_minutes < 1) {
                _config->_num_minutes = 1;
            }
            else if (_config->_num_minutes > 255) {
                _config->_num_minutes = 255;
            }
            logu_("Setting match minutes to: %d\n", _config->_num_minutes);
            BYTE *cs = get_target_location2(_config->_hp_at_call_set_minutes);
            patch_set_minutes[3] = _config->_num_minutes;
            patch_at_location(cs, patch_set_minutes, sizeof(patch_set_minutes)-1);
        }
    }

    return result;
}

void init_direct_input()
{
    if (!_config->_overlay_enabled) {
        return;
    }

    // initialize DirectInput
    g_IDirectInput8 = NULL;
    if (SUCCEEDED(DirectInput8Create(
        myHDLL, DIRECTINPUT_VERSION, IID_IDirectInput8,
        (void**)&g_IDirectInput8, NULL))) {
        logu_("g_IDirectInput8 = %p\n", g_IDirectInput8);

        // hook CreateDevice
        HookVtblMethod2(g_IDirectInput8, 3, (void**)&_org_CreateDevice, NULL, sider_CreateDevice, NULL, "IDirectInput8::CreateDevice");
    }
    else {
        logu_("PROBLEM creating DirectInput interface\n");
    }

    if (!_gamepad_config->_dinput_enabled) {
        return;
    }
}

void enumerate_controllers()
{
    _enumerated_controllers = true;
    if (!_gamepad_config->_dinput_enabled) {
        return;
    }

    // enumerate devices
    _has_controller = false;
    logu_("Enumerating game controllers\n");
    if (SUCCEEDED(g_IDirectInput8->EnumDevices(
        DI8DEVCLASS_GAMECTRL, sider_device_enum_callback, NULL, DIEDFL_ALLDEVICES))) {
        logu_("Done enumerating game controllers\n");

        if (_has_controller) {
            g_IDirectInputDevice8 = NULL;
            if (SUCCEEDED(g_IDirectInput8->CreateDevice(
                g_controller_guid_instance, &g_IDirectInputDevice8, NULL))) {
                logu_("DirectInputDevice created: %p\n", g_IDirectInputDevice8);

                // enumerate buttons and prepare data format
                if (SUCCEEDED(g_IDirectInputDevice8->EnumObjects(
                    sider_object_enum_callback, NULL, DIDFT_PSHBUTTON | DIDFT_AXIS | DIDFT_POV))) {
                    logu_("number of inputs: %d\n", _di_objects.size());

                    memset(&_data_format, 0, sizeof(DIDATAFORMAT));
                    _data_format.dwSize = sizeof(DIDATAFORMAT);
                    _data_format.dwObjSize = sizeof(DIOBJECTDATAFORMAT);
                    _data_format.dwFlags = DIDF_ABSAXIS;
                    _data_format.dwDataSize = sizeof(_controller_buttons);
                    _data_format.dwNumObjs = _di_objects.size();
                    size_t rgodf_size = sizeof(DIOBJECTDATAFORMAT) * _di_objects.size();
                    _data_format.rgodf = (LPDIOBJECTDATAFORMAT)malloc(rgodf_size);
                    int i = 0;
                    vector<DIDEVICEOBJECTINSTANCE>::iterator it;
                    for (it = _di_objects.begin(); it != _di_objects.end(); it++, i++) {
                        _data_format.rgodf[i].pguid = &it->guidType;
                        _data_format.rgodf[i].dwOfs = it->dwOfs;
                        _data_format.rgodf[i].dwType = it->dwType;
                        _data_format.rgodf[i].dwFlags = 0;
                    }

                    _controller_prepped = false;
                    memset(_controller_buttons, 0, sizeof(_controller_buttons));
                    memset(_prev_controller_buttons, 0, sizeof(_prev_controller_buttons));
                }
            }
        }
    }
    else {
        logu_("PROBLEM enumerating game controllers\n");
    }
}

INT APIENTRY DllMain(HMODULE hDLL, DWORD Reason, LPVOID Reserved)
{
    wstring *match = NULL;
    INT result = FALSE;
    HWND main_hwnd;
    ULONGLONG s,f;

    switch(Reason) {
        case DLL_PROCESS_ATTACH:
            s = GetTickCount64();

            myHDLL = hDLL;
            memset(module_filename, 0, sizeof(module_filename));
            if (GetModuleFileName(NULL, module_filename, MAX_PATH)==0) {
                return FALSE;
            }
            if (!init_paths()) {
                return FALSE;
            }
            //log_(L"DLL_PROCESS_ATTACH: %s\n", module_filename);
            if (skip_process(module_filename)) {
                return FALSE;
            }

            read_configuration(_config);

            if (is_sider(module_filename)) {
                _is_sider = true;
                return TRUE;
            }

            if (is_pes(module_filename, &match)) {
                read_gamepad_global_mapping(_gamepad_config);

                wstring version;
                get_module_version(hDLL, version);
                start_log_(L"============================\n");
                log_(L"Sider DLL: version %s\n", version.c_str());
                log_(L"Filename match: %s\n", match->c_str());
                if (is_already_loaded(hDLL)) {
                    log_(L"DLL already loaded into the game. Nothing to do\n");
                    close_log_();
                    return FALSE;
                }

                // set up a thread-scope hook, to replace global hook
                // so that we stay mapped in game process
                setHook1();

                _overlay_on = _config->_overlay_on_from_start;
                _overlay_header = L"sider ";
                _overlay_header += version;
                memset(_overlay_text, 0, sizeof(_overlay_text));
                memset(_current_overlay_text, 0, sizeof(_current_overlay_text));
                memset(_overlay_utf8_text, 0, sizeof(_overlay_utf8_text));
                memset(_overlay_utf8_image_path, 0, sizeof(_overlay_utf8_image_path));
                memset(&_overlay_image, 0, sizeof(overlay_image_t));

                install_func(NULL);

                f = GetTickCount64();
                log_(L"Initialized in %0.3f seconds\n", (f - s)/1000.0);

                // initialize audio lib
                audio_init();

                /**
                // performance test
                char *str = "UTF-8 check: ленинградское время ноль часов ноль минут.";
                wchar_t *wstr = Utf8::utf8ToUnicode(str);
                logu_("Performance test. UTF-8 str   : %s\n", str);
                log_(L"Performance test. Unicode str : %s\n", wstr);

                DWORD s1 = GetTickCount();
                for (int i=0; i<1000000; i++) {
                    wchar_t *w = Utf8::utf8ToUnicode(str);
                    Utf8::free(w);
                }
                DWORD f1 = GetTickCount();
                DWORD s2 = GetTickCount();
                for (int i=0; i<1000000; i++) {
                    char *s = Utf8::unicodeToUtf8(wstr);
                    Utf8::free(s);
                }
                DWORD f2 = GetTickCount();
                logu_("Utf8: %d, %d\n", f1-s1, f2-s2);

                DWORD s3 = GetTickCount();
                for (int i=0; i<1000000; i++) {
                    wchar_t *w = Utf8org::utf8ToUnicode((BYTE*)str);
                    Utf8org::free((BYTE*)w);
                }
                DWORD f3 = GetTickCount();
                DWORD s4 = GetTickCount();
                for (int i=0; i<1000000; i++) {
                    char *s = (char*)Utf8org::unicodeToUtf8(wstr);
                    Utf8org::free(s);
                }
                DWORD f4 = GetTickCount();
                logu_("Utf8org: %d, %d\n", f3-s3, f4-s4);
                **/

                // tell sider.exe to unhook CBT and quit
                if (!_config->_start_game.empty()) {
                    HWND main_hwnd = FindWindow(SIDERCLS, NULL);
                    if (main_hwnd) {
                        PostMessage(main_hwnd, SIDER_MSG_EXIT, 0, 0);
                        log_(L"Posted message for sider.exe to quit\n");
                    }
                }

                delete match;
                return TRUE;
            }

            return result;
            break;

        case DLL_PROCESS_DETACH:
            //log_(L"DLL_PROCESS_DETACH: %s\n", module_filename);

            if (_is_game) {
                log_(L"DLL detaching from (%s).\n", module_filename);
                log_(L"Unmapping from PES.\n");

                if (_controller_poll_handle != INVALID_HANDLE_VALUE) {
                    log_(L"Waiting for controller poll thread to finish ...\n");
                    _controller_poll = false;
                    DWORD res = WaitForSingleObject(_controller_poll_handle, 1000);
                    log_(L"Wait is over: %08x\n", res);
                }

                if (g_IDirectInputDevice8) {
                    g_IDirectInputDevice8->Unacquire();
                    log_(L"Releasing DirectInputDevice interface (%p)\n", g_IDirectInputDevice8);
                    g_IDirectInputDevice8->Release();
                    g_IDirectInputDevice8 = NULL;
                }
                if (g_IDirectInput8) {
                    log_(L"Releasing DirectInput interface (%p)\n", g_IDirectInput8);
                    g_IDirectInput8->Release();
                    g_IDirectInput8 = NULL;
                }

                if (L) { lua_close(L); }
                log_(L"trophy-table copy count: %lld\n", _trophy_table_copy_count);

                //if (_lookup_cache) {
                //    log_(L"lookup_cache:: size = %d\n", _lookup_cache->size());
                //    delete _lookup_cache;
                //}

                //if (_key_cache) { delete _key_cache; }
                if (_small_key_cache) { delete _small_key_cache; }
                //if (_rewrite_cache) { delete _rewrite_cache; }

#ifdef PERF_TESTING
                double fileops_tc = _fileops_stats->_total_count;
                double miss_tc = _stats->_total_count + _content_stats->_total_count;

                uint64_t tps = 0;
                QueryPerformanceFrequency((LARGE_INTEGER*)&tps);
                log_(L"ticks per second: %llu\n", tps);

                if (_stats) { delete _stats; }
                if (_content_stats) { delete _content_stats; }
                if (_fileops_stats) { delete _fileops_stats; }
                if (_overlay_stats) { delete _overlay_stats; }

                // report hit pc
                if (fileops_tc > 1.0) {
                    double hit_pct = 100*(1.0 - miss_tc / fileops_tc);
                    log_(L"cache hit pct: %0.1f%%\n", hit_pct);
                }
#endif

                // tell sider.exe to close
                if (_config->_close_sider_on_exit || !_config->_start_game.empty()) {
                    main_hwnd = FindWindow(SIDERCLS, NULL);
                    if (main_hwnd) {
                        PostMessage(main_hwnd, SIDER_MSG_EXIT, 0, 0);
                        log_(L"Posted message for sider.exe to quit\n");
                    }
                }
                DeleteCriticalSection(&_cs);
                DeleteCriticalSection(&_tcs);

                if (handle1) {
                    UnhookWindowsHookEx(handle1);
                }

                log_(L"All done.\n");
                close_log_();
            }
            break;

        case DLL_THREAD_ATTACH:
            //log_(L"DLL_THREAD_ATTACH: %s\n", module_filename);
            break;

        case DLL_THREAD_DETACH:
            //log_(L"DLL_THREAD_DETACH: %s\n", module_filename);
            break;

    }

    return TRUE;
}

LRESULT CALLBACK sider_foreground_idle_proc(int code, WPARAM wParam, LPARAM lParam) {
    return CallNextHookEx(handle1, code, wParam, lParam);
}

void play_overlay_toggle_sound()
{
    if (_config->_overlay_toggle_sound != L"") {
        wstring fname(sider_dir);
        fname += _config->_overlay_toggle_sound;
        char *utf8filename = Utf8::unicodeToUtf8(fname.c_str());

        sound_t* sound = audio_new_sound(utf8filename, NULL);
        if (sound) {
            audio_set_volume(sound, _config->_overlay_toggle_sound_volume);
            audio_play(sound);
        }
        Utf8::free(utf8filename);
    }
}

LRESULT CALLBACK sider_keyboard_proc(int code, WPARAM wParam, LPARAM lParam)
{
    if (code < 0) {
        return CallNextHookEx(kb_handle, code, wParam, lParam);
    }

    if (code == HC_ACTION) {
        if (!(_block_input && _hard_block) && wParam == _config->_overlay_vkey_toggle && ((lParam & 0x80000000) != 0)) {
            _overlay_on = !_overlay_on;
            play_overlay_toggle_sound();
            sider_dispatch_show_hide_events(_overlay_on);
            DBG(64) logu_("overlay: %s\n", (_overlay_on)?"ON":"OFF");
            if (_overlay_on) {
                _overlay_image.to_clear = true;
            }
        }
        else if (!(_block_input && _hard_block) && wParam == _config->_vkey_reload_2 && ((lParam & 0x80000000) != 0)) {
            if (_reload_1_down) {
                _reload_modified = true;
            }
        }
        else if (!(_block_input && _hard_block) && wParam == _config->_vkey_reload_1 && ((lParam & 0x80000000) == 0)) {
            _reload_1_down = ((lParam & 0x80000000) == 0);
        }

        if (_overlay_on) {
            //logu_("sider_keyboard_proc: wParam=%p, lParam=%p\n", wParam, lParam);
            // deliver keyboard event to module
            if (_config->_lua_enabled && ((lParam & 0x80000000) == 0)) {
                if (_curr_overlay_m != _modules.end()) {
                    // module switching keys
                    // "[" - 0xdb, "]" - 0xdd, "~" - 0xc0, "1" - 0x31
                    if (!(_block_input && _hard_block) && wParam == _config->_overlay_vkey_next_module) {
                        sider_switch_overlay_to_next_module();
                    }
                    else if (!(_block_input && _hard_block) && wParam == _config->_overlay_vkey_prev_module) {
                        sider_switch_overlay_to_prev_module();
                    }
                    else {
                        // lua callback
                        module_t *m = *_curr_overlay_m;
                        module_key_down(m, (int)wParam);
                    }
                }
            }
            else if (_config->_lua_enabled && ((lParam & 0x80000000) != 0)) {
                if (_curr_overlay_m != _modules.end()) {
                    // lua callback
                    module_t *m = *_curr_overlay_m;
                    module_key_up(m, (int)wParam);
                }
            }
        }
    }
    return CallNextHookEx(kb_handle, code, wParam, lParam);
}

LRESULT CALLBACK meconnect(int code, WPARAM wParam, LPARAM lParam)
{
    if (hookingThreadId == GetCurrentThreadId()) {
        log_(L"called in hooking thread!\n");
    }
    return CallNextHookEx(handle, code, wParam, lParam);
}

void setHook()
{
    handle = SetWindowsHookEx(WH_CBT, meconnect, myHDLL, 0);
    log_(L"------------------------\n");
    log_(L"handle = %p\n", handle);
}

void setHook1()
{
    handle1 = SetWindowsHookEx(WH_FOREGROUNDIDLE, sider_foreground_idle_proc, myHDLL, GetCurrentThreadId());
    log_(L"handle1 = %p\n", handle1);
}

void unsetHook(bool all)
{
    open_log_(L"windows hooks: handle = %p\n", handle);
    UnhookWindowsHookEx(handle);
    log_(L"windows hooks: CBT unhooked\n");
    if (all && kb_handle) {
        UnhookWindowsHookEx(kb_handle);
        log_(L"windows hooks: keyboard unhooked\n");
    }
    close_log_();
}

bool get_start_game(wstring &start_game) {
    if (_config && !_config->_start_game.empty()) {
        start_game = _config->_start_game;
        return true;
    }
    start_game = L"";
    return false;
}
