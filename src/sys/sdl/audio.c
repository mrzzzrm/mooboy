#include "audio.h"
#include "core/sound.h"
#include "sys/sys.h"
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
    sys_lock_audiobuf();
    if(sound.buf_start > sound.buf_end) {
        r = sound.buf_size - (sound.buf_start - sound.buf_end) + 1;
    }
    else {
        r = sound.buf_end - sound.buf_start;
    }
    sys_unlock_audiobuf();

    return r;
}

static void handout_buf(void *_unused, Uint8 *stream, int length) {
    sys_lock_audiobuf();
    u16 requested_samples = length / (sound.sample_size * 2);
    u16 available_samples;
    u16 served_samples;

    while((available_samples = get_available_samples()) < requested_samples) {
        sound_mix();
    }

    if(sound.buf_start > sound.buf_end) {
        served_samples = sound.buf_size - sound.buf_start;
        if(served_samples >= requested_samples) {
            memcpy(stream, &sound.buf[sound.buf_start*2*2], length);
        }
        else {
            u16 bytes = served_samples * sound.sample_size * 2;
            memcpy(&stream[0], &sound.buf[sound.buf_start*2*2], bytes);
            memcpy(&stream[bytes], &sound.buf[0], (requested_samples - served_samples) * sound.sample_size * 2);
        }
    }
    else {
        memcpy(stream, &sound.buf[sound.buf_start*2*2], length);
    }


    sound.buf_start += requested_samples;
    sound.buf_start %= sound.buf_size;
    sys_unlock_audiobuf();
}


void audio_init() {
    SDL_AudioSpec format;

    format.freq = sound.freq;
    format.format = AUDIO_S16;
    format.channels = 2;
    format.samples = 512;
    format.callback = handout_buf;
    format.userdata = NULL;

    mutex = SDL_CreateMutex();

    if (SDL_OpenAudio(&format, NULL) < 0 ) {
        fprintf(stderr, "Couldn't open audio device: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_PauseAudio(0);
}


