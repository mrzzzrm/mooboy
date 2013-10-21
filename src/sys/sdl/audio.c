#include "audio.h"
#include "core/sound.h"
#include "core/moo.h"
#include "sys/sys.h"
#include <assert.h>
#include <SDL/SDL.h>

static SDL_mutex *mutex;


void sys_lock_audiobuf() {
    SDL_mutexP(mutex);
}

void sys_unlock_audiobuf() {
    SDL_mutexV(mutex);
}

static u16 get_available_samples() {
    u16 r;
    if(sys.sound_buf_start > sys.sound_buf_end) {
        r = sys.sound_buf_size - (sys.sound_buf_start - sys.sound_buf_end) + 1;
    }
    else {
        r = sys.sound_buf_end - sys.sound_buf_start;
    }

    return r;
}

static void handout_buf(void *_unused, Uint8 *stream, int length) {
    u16 requested_samples = length / (sys.sound_sample_size * 2);
    u16 available_samples;
    u16 served_samples;

    sys_lock_audiobuf();

    if(!sys.sound_on || (~moo.state & MOO_ROM_RUNNING_BIT)) {
        memset(stream, 0x00, length);
    }
    else {
#ifdef DEBUG
        if(get_available_samples() < requested_samples) {
      //      printf("WARNING: Sound buffer underrun\n");
        }
#endif

        while((available_samples = get_available_samples()) < requested_samples) {
            sound_mix();
        }

        if(sys.sound_buf_start > sys.sound_buf_end) {
            served_samples = sys.sound_buf_size - sys.sound_buf_start;
            if(served_samples >= requested_samples) {
                memcpy(stream, &sys.sound_buf[sys.sound_buf_start*2*2], length);
            }
            else {
                u16 bytes = served_samples * sys.sound_sample_size * 2;
                memcpy(&stream[0], &sys.sound_buf[sys.sound_buf_start*2*2], bytes);
                memcpy(&stream[bytes], &sys.sound_buf[0], (requested_samples - served_samples) * sys.sound_sample_size * 2);
            }
        }
        else {
            memcpy(stream, &sys.sound_buf[sys.sound_buf_start*2*2], length);
        }
    }

    sys.sound_buf_start += requested_samples;
    sys.sound_buf_start %= sys.sound_buf_size;

    sys_unlock_audiobuf();
}

void audio_init() {
    SDL_AudioSpec format;

    format.freq = sys.sound_freq;
    format.format = AUDIO_S16;
    format.channels = 2;
    format.samples = 512;
    format.callback = handout_buf;
    format.userdata = NULL;

    mutex = SDL_CreateMutex();

    if (SDL_OpenAudio(&format, NULL) < 0 ) {
        moo_fatalf("Couldn't open audio device: %s", SDL_GetError());
        exit(1);
    }
    SDL_PauseAudio(1);
}


