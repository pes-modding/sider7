#ifndef _SIDER_AUDIO_H
#define _SIDER_AUDIO_H

#include "lua.hpp"
#include "lauxlib.h"
#include "lualib.h"

namespace Audio {
    enum State {
        created = 0,
        playing,
        pausing,
        paused,
        finished,
    };
}

struct sound_t;

void init_audio_lib(lua_State *L);

// C API
void audio_init();
sound_t* audio_new_sound(const char* filename, sound_t* sound);
int audio_play(sound_t* sound);
int audio_pause(sound_t* sound);
int audio_finish(sound_t* sound);
int audio_set_volume(sound_t* sound, float volume);

#endif
