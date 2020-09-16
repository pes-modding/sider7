#define UNICODE

#define DR_FLAC_IMPLEMENTATION
#include "extras/dr_flac.h"  /* Enables FLAC decoding. */
#define DR_MP3_IMPLEMENTATION
#include "extras/dr_mp3.h"   /* Enables MP3 decoding. */
#define DR_WAV_IMPLEMENTATION
#include "extras/dr_wav.h"   /* Enables WAV decoding. */

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "audio.h"
#include "common.h"
#include "config.h"
#include "sider.h"
#include "utf8.h"

#include <deque>

extern config_t*_config;
extern void module_call_callback_with_context(lua_State *L, lua_State *from_L, int callback_index);
extern CRITICAL_SECTION _cs;

#define DBG(n) if (_config->_debug & n)

struct sound_t {
    const char *filename;
    ma_device* pDevice;
    ma_decoder* pDecoder;
    int state;
    bool fading;
    bool fade_and_pause;
    bool fade_and_finish;
    float fade_to_volume;
    ma_uint32 fade_in_samples;
    lua_State *L;
    void* extra;
};

struct extra_info_t {
    int callback_index;
};

lua_State *_audio_L;
int _audio_objects_index = 0;

class sound_tracker_t {
    std::deque<sound_t*> _dq;
    CRITICAL_SECTION* _pcs;
public:
    sound_tracker_t(CRITICAL_SECTION* pcs) : _pcs(pcs) {}
    ~sound_tracker_t() {}
    void add(sound_t* p) {
        lock_t lock(_pcs);
        _dq.push_back(p);
        logu_("added sound object %p\n", p);
    }
    void check() {
        size_t sz;
        {
            lock_t lock(_pcs);
            sz = _dq.size();
        }
        DBG(8192) logu_("checking sound objects (%d)\n", sz);
        for (int i=0; i<sz; i++) {
            lock_t lock(_pcs);
            std::deque<sound_t*>::iterator it = _dq.begin();
            sound_t *p = *it;
            _dq.pop_front();
            switch (p->state) {
                case Audio::finished:
                    // signalled: remove and destroy
                    logu_("sound object %p is done\n", p);
                    // check if there is a callback then call it
                    if (p->extra) {
                        extra_info_t* info = (extra_info_t*)p->extra;
                        if ((p->L) && (info->callback_index > 0)) {
                            module_call_callback_with_context(p->L, _audio_L, info->callback_index);
                        }
                    }
                    if (p->pDevice) {
                        ma_device_uninit(p->pDevice);
                        free(p->pDevice);
                        p->pDevice = NULL;
                    }
                    if (p->pDecoder) {
                        ma_decoder_uninit(p->pDecoder);
                        free(p->pDecoder);
                        p->pDecoder = NULL;
                    }
                    // drop from the tracking table to allow eventual garbage collection
                    if (p->L) {
                        lua_pushlightuserdata(p->L, p);
                        lua_pushnil(p->L); // objects[ptr] = nil
                        lua_xmove(p->L, _audio_L, 2);
                        lua_settable(_audio_L, _audio_objects_index);
                    }
                    break;
                case Audio::pausing:
                    logu_("sound object %p is paused\n", p);
                    p->state = Audio::paused;
                    if (p->pDevice) {
                        ma_device_stop(p->pDevice);
                    }
                    // fall through to default, as we need to re-enqueue still
                default:
                    // re-enqueue
                    _dq.push_back(p);
                    DBG(8192) logu_("sound object %p is not done yet\n", p);
            }
        }
    }
};

static sound_tracker_t* _sound_tracker(NULL);
static HANDLE _sound_manager_handle(INVALID_HANDLE_VALUE);

static DWORD sound_manager_thread(LPVOID param)
{
    bool done(false);
    sound_tracker_t* tracker = (sound_tracker_t*)param;
    logu_("sound-manager thread started\n");
    while (!done) {
        tracker->check();
        Sleep(1000);
    }
    logu_("sound-manager thread finished\n");
    return 0;
}

static void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    sound_t* p = (sound_t*)pDevice->pUserData;
    if (!p || p->pDecoder == NULL) {
        logu_("warning: sound object %p : pDevice=%p, pDecoder=%p\n", p, pDevice, p->pDecoder);
        return;
    }

    ma_uint64 n = ma_decoder_read_pcm_frames(p->pDecoder, pOutput, frameCount);
    //logu_("read %llu frames\n", n);

    if (n == 0 && p->state != Audio::finished) {
        logu_("all samples played for sound object %p\n", p);
        p->state = Audio::finished;
    }

    if (p->fading) {
        float volume;
        if (ma_device_get_master_volume(pDevice, &volume) == MA_SUCCESS) {
            if (p->fade_in_samples <= frameCount) {
                // close enough
                volume = p->fade_to_volume;
                p->fading = false;
                p->fade_in_samples = 0;
                logu_("%llu: sound object %p finished fading\n", GetTickCount64(), p);
                if (p->fade_and_finish) {
                    p->state = Audio::finished;
                }
                else if (p->fade_and_pause) {
                    p->state = Audio::pausing;
                }
            }
            else {
                float rem_change = p->fade_to_volume - volume;
                float volume_change = rem_change * (float)frameCount / p->fade_in_samples;
                volume += volume_change;
                p->fade_in_samples -= frameCount;
            }
            //logu_("fading sound object %p. (%d,%0.3f) volume now: %0.3f\n", p, frameCount, p->fade_in_samples, volume);
            ma_device_set_master_volume(pDevice, volume);
        }
    }

    (void)pInput;
}

void audio_init()
{
    if (_sound_tracker == NULL) {
        _sound_tracker = new sound_tracker_t(&_cs);
        DWORD thread_id;
        _sound_manager_handle = CreateThread(NULL, 0, sound_manager_thread, _sound_tracker, 0, &thread_id);
        SetThreadPriority(_sound_manager_handle, THREAD_PRIORITY_LOWEST);
    }
}

sound_t* audio_new_sound(const char *filename, sound_t *sound)
{
    ma_result result;
    ma_decoder* pDecoder;
    ma_device* pDevice;
    ma_device_config deviceConfig;

    pDevice = (ma_device*)malloc(sizeof(ma_device));
    pDecoder = (ma_decoder*)malloc(sizeof(ma_decoder));
    if (sound == NULL) {
        sound = (sound_t*)malloc(sizeof(sound_t));
    }

    wchar_t *wfname = Utf8::utf8ToUnicode(filename);
    if (!wfname) {
        logu_("utf8-to-wide conversion failed for: %s\n", filename);
        Utf8::free(wfname);
        return NULL;
    }

    result = ma_decoder_init_file_w(wfname, NULL, pDecoder);
    if (result != MA_SUCCESS) {
        logu_("ma_decoder_init_file failed for: %s\n", filename);
        //free(sound);
        Utf8::free(wfname);
        return NULL;
    }
    Utf8::free(wfname);

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = pDecoder->outputFormat;
    deviceConfig.playback.channels = pDecoder->outputChannels;
    deviceConfig.sampleRate        = pDecoder->outputSampleRate;
    deviceConfig.dataCallback      = data_callback;
    deviceConfig.pUserData         = sound;

    if (ma_device_init(NULL, &deviceConfig, pDevice) != MA_SUCCESS) {
        logu_("Failed to open playback device.\n");
        ma_decoder_uninit(pDecoder);
        free(pDecoder);
        //free(sound);
        return NULL;
    }

    sound->filename = strdup(filename);
    sound->pDecoder = pDecoder;
    sound->pDevice = pDevice;
    sound->state = Audio::created;
    sound->fading = false;
    sound->fade_and_pause = false;
    sound->fade_and_finish = false;
    sound->fade_to_volume = 0.0f;
    sound->fade_in_samples = 0;
    sound->L = NULL;
    sound->extra = NULL;
    return sound;
}

int audio_play(sound_t* sound) {
    if (!sound || !sound->pDevice) {
        return -1;
    }
    switch (sound->state) {
        case Audio::created:
            _sound_tracker->add(sound);
        case Audio::paused:
            if (ma_device_start(sound->pDevice) != MA_SUCCESS) {
                sound->state = Audio::finished;
                return -2;
            }
            break;
        case Audio::finished:
            return 0;
    }
    sound->state = Audio::playing;
    logu_("playing: %s\n", sound->filename);
    return 0;
}

int audio_pause(sound_t* sound) {
    if (!sound || !sound->pDevice) {
        return -1;
    }
    switch (sound->state) {
        case Audio::playing:
            if (ma_device_stop(sound->pDevice) != MA_SUCCESS) {
                sound->state = Audio::finished;
                return -2;
            }
            break;
        case Audio::finished:
            return 0;
    }
    sound->state = Audio::paused;
    logu_("paused: %s\n", sound->filename);
    return 0;
}

int audio_finish(sound_t* sound) {
    if (!sound || !sound->pDevice) {
        return -1;
    }
    switch (sound->state) {
        case Audio::playing:
            if (ma_device_stop(sound->pDevice) != MA_SUCCESS) {
                sound->state = Audio::finished;
                return -2;
            }
            break;
    }
    sound->state = Audio::finished;
    logu_("finished: %s\n", sound->filename);
    return 0;
}

int audio_set_volume(sound_t *sound, float volume) {
    if (sound && sound->pDevice) {
        if (sound->state != Audio::finished) {
            ma_device_set_master_volume(sound->pDevice, volume);
            return 0;
        }
    }
    return -1;
}

static sound_t* checksound(lua_State *L) {
    void *ud = luaL_checkudata(L, 1, "Sider.sound");
    luaL_argcheck(L, ud != NULL, 1, "'sound' expected");
    return (sound_t*)ud;
}

static int audio_lua_set_volume(lua_State *L)
{
    lock_t lock(&_cs);
    sound_t* sound = checksound(L);
    double volume = luaL_checknumber(L, 2);
    if (volume < 0.0) {
        volume = 0.0;
    }
    if (volume > 1.0) {
        volume = 1.0;
    }
    if (sound->pDevice) {
        ma_device_set_master_volume(sound->pDevice, volume);
    }
    lua_pop(L, lua_gettop(L));
    return 0;
}

static int audio_lua_get_volume(lua_State *L)
{
    lock_t lock(&_cs);
    sound_t* sound = checksound(L);
    lua_pop(L, lua_gettop(L));
    float volume = 0.0f;
    if (sound->pDevice) {
        ma_device_get_master_volume(sound->pDevice, &volume);
    }
    lua_pushnumber(L, volume);
    return 1;
}

static int audio_lua_get_filename(lua_State *L)
{
    lock_t lock(&_cs);
    sound_t* sound = checksound(L);
    lua_pop(L, lua_gettop(L));
    lua_pushstring(L, sound->filename);
    return 1;
}

static int audio_lua_fade_to(lua_State *L)
{
    lock_t lock(&_cs);
    sound_t* sound = checksound(L);
    if (!sound->pDevice) {
        lua_pop(L, lua_gettop(L));
        return 0;
    }
    float fade_to_volume = luaL_checknumber(L, 2);
    if (fade_to_volume > 1.0f) {
        fade_to_volume = 1.0f;
    }
    else if (fade_to_volume < 0.0f) {
        fade_to_volume = 0.0f;
    }
    float fade_sec = 1.0; // default time: 1 sec
    if (lua_isnumber(L, 3)) {
        fade_sec = lua_tonumber(L, 3);
        if (fade_sec < 0.001f) {
            fade_sec = 0.001f;
        }
    }
    if (sound->state != Audio::finished) {
        sound->fading = true;
        sound->fade_to_volume = fade_to_volume;
        sound->fade_in_samples = fade_sec * sound->pDevice->sampleRate;
        logu_("%llu: fading sound object %p\n", GetTickCount64(), sound);
    }
    lua_pop(L, lua_gettop(L));
    return 0;
}

static int audio_lua_play(lua_State *L)
{
    lock_t lock(&_cs);
    sound_t* sound = checksound(L);
    if (!sound->pDevice) {
        lua_pop(L, lua_gettop(L));
        return 0;
    }
    // play
    if (audio_play(sound) != 0) {
        luaL_error(L, "problem playing sound object %p", sound);
    }
    sound->fade_and_pause = false;
    sound->fade_and_finish = false;
    lua_pop(L, lua_gettop(L));
    return 0;
}

static int audio_lua_pause(lua_State *L)
{
    lock_t lock(&_cs);
    sound_t* sound = checksound(L);
    if (!sound->pDevice) {
        lua_pop(L, lua_gettop(L));
        return 0;
    }
    if (!sound->fading) {
        // pause now
        if (audio_pause(sound) != 0) {
            lua_pop(L, lua_gettop(L));
            luaL_error(L, "problem pausing sound object %p", sound);
        }
        return 0;
    }
    sound->fade_and_pause = true;
    sound->fade_and_finish = false;
    lua_pop(L, lua_gettop(L));
    return 0;
}

static int audio_lua_finish(lua_State *L)
{
    lock_t lock(&_cs);
    sound_t* sound = checksound(L);
    if (!sound->pDevice) {
        lua_pop(L, lua_gettop(L));
        return 0;
    }
    if (!sound->fading) {
        // finish now
        if (audio_finish(sound) != 0) {
            lua_pop(L, lua_gettop(L));
            luaL_error(L, "problem finishing sound object %p", sound);
        }
        return 0;
    }
    sound->fade_and_pause = false;
    sound->fade_and_finish = true;
    lua_pop(L, lua_gettop(L));
    return 0;
}

static int audio_lua_when_done(lua_State *L)
{
    lock_t lock(&_cs);
    sound_t* sound = checksound(L);
    if (!sound || !sound->pDevice) {
        lua_pop(L, lua_gettop(L));
        luaL_error(L, "sound object does not exist");
    }

    if (!lua_isfunction(L, 2)) {
        lua_pop(L, lua_gettop(L));
        luaL_error(L, "expecting a function");
    }

    lua_pushvalue(L, 2);
    lua_xmove(L, _audio_L, 1); // store callback function on the stack too

    extra_info_t* info = (extra_info_t*)malloc(sizeof(extra_info_t));
    info->callback_index = lua_gettop(_audio_L);
    logu_("info->callback_index = %d\n", info->callback_index);

    sound->extra = info;
    lua_pop(L, lua_gettop(L));
    return 0;
}

static int audio_lua_new(lua_State *L)
{
    lock_t lock(&_cs);
    size_t len = 0;
    const char *filename = luaL_checklstring(L, 1, &len);
    if (!filename || len == 0) {
        lua_pop(L, 1);
        lua_pushnil(L);
        lua_pushstring(L, "filename cannot be empty");
        return 2;
    }
    lua_pop(L, 1);

    sound_t* sound = (sound_t*)lua_newuserdata(L, sizeof(sound_t));
    sound = audio_new_sound(filename, sound);
    if (!sound) {
        lua_pushnil(L);
        lua_pushfstring(L, "unable to create new sound object for: %s", filename);
        return 2;
    }
    sound->L = L;

    luaL_getmetatable(L, "Sider.sound");
    lua_setmetatable(L, -2);

    // keep a reference to the sound object so that
    // it is not garbage-collected, until it finished playing
    lua_pushlightuserdata(L, sound);
    lua_pushvalue(L, 1);
    lua_xmove(L, _audio_L, 2); // key/value pair: userdata --> true function
    lua_settable(_audio_L, _audio_objects_index);

    // 2nd return value: no error
    lua_pushnil(L);
    return 2;
}

static const struct luaL_Reg audiolib_f [] = {
    {"new", audio_lua_new},
    {NULL, NULL}
};

static const struct luaL_Reg audiolib_m [] = {
    {"play", audio_lua_play},
    {"pause", audio_lua_pause},
    {"finish", audio_lua_finish},
    {"set_volume", audio_lua_set_volume},
    {"get_volume", audio_lua_get_volume},
    {"get_filename", audio_lua_get_filename},
    {"fade_to", audio_lua_fade_to},
    {"when_done", audio_lua_when_done},
    {NULL, NULL}
};

void init_audio_lib(lua_State *L)
{
    lock_t lock(&_cs);
    // keep new state anchored.
    // we will keep callbacks on its stack
    _audio_L = lua_newthread(L);
    lua_newtable(_audio_L);
    _audio_objects_index = lua_gettop(_audio_L);

    luaL_newmetatable(L, "Sider.sound");

    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);  /* pushes the metatable */
    lua_settable(L, -3);  /* metatable.__index = metatable */

    luaL_openlib(L, NULL, audiolib_m, 0);

    luaL_openlib(L, "audio", audiolib_f, 0);
}
