#include "sound.h"
#include "cpu.h"
#include <SDL/SDL.h>

sound_t sound;
ch1_t ch1;
sweep_t sweep;
env_t env1;

static SDL_mutex *mutex;

    static long moved = 0;
    static long mixed = 0;


static u16 sweep_calc() {
    u16 tmp = sweep.shadow >> sweep.shift;
    if(sweep.negate) {
        tmp = ~tmp;
    }
    if(sweep.shadow + tmp > 0x0800) {
        ch1.enabled = 0;
    }
    return sweep.shadow + tmp;
}

static void ch1_trigger() {
//    if(ch1.length_counter == 0) {
//        length_counter = 0x40;
//    }

    // Sweep
    sweep.shadow = ch1.freq;
    sweep.enable = sweep.period || sweep.shift;

    if(sweep.shift != 0) {
        sweep.shadow = sweep_calc();
    }
}

static void tick_length_counter() {

}

static void tick_sweep() {
    if(sweep.enable && sweep.period != 0) {
        u16 tmp = sweep_calc();
        if(tmp < 0x0800 && sweep.shift != 0) {
           sweep.shadow = tmp;
           ch1.freq = tmp;
           sweep_calc();
        }
    }
}

static void tick_envelope() {
    if(env1.period == 0) {
        return;
    }

    if(env1.mode) {
        if(ch1.volume < 0x0F) {
            ch1.volume++;
        }
    }
    else {
        if(ch1.volume > 0x00) {
            ch1.volume--;
        }
    }
 }

static void timer_step() {
    u8 step = (cpu.cc >> 11) & 0x07;
    if(step == sound.last_timer_step) {
        return;
    }
    else {
        sound.last_timer_step = step;
    }

    switch(step) {
        case 0: tick_length_counter();               break;
        case 2: tick_length_counter(); tick_sweep(); break;
        case 4: tick_length_counter();               break;
        case 6: tick_length_counter(); tick_sweep(); break;
        case 7: tick_envelope();                     break;
    }
}

static void ch1_mix() {
    if((sound.buf_end + 1) % sound.buf_size == sound.buf_start) {
        printf("Overrun!\n");
        return;
    }

    u32 cursor;
    static size_t c = 0;
    unsigned int i, framesize, framecount;
    Uint16 *stream = (Uint16*)sound.buf;
    framesize = 2;
    int wavelen = 44100.0f / (double)(131072/(2048-ch1.freq));

    cursor = sound.buf_end * sound.sample_size;

    if(!sound.enabled ) {
        sound.buf[cursor + 0] = 0;
        sound.buf[cursor + 1] = 0;
        return;
    }

    u16 val;
    int in_wave = c%wavelen;

    switch(ch1.duty) {
        case 0x00: val = (float)in_wave <= (float)wavelen*0.125 ? 0x0000 : 0x3FF; break;
        case 0x01: val = (float)in_wave <= (float)wavelen*0.250 ? 0x0000 : 0x3FF; break;
        case 0x02: val = (float)in_wave <= (float)wavelen*0.500 ? 0x0000 : 0x3FF; break;
        case 0x03: val = (float)in_wave <= (float)wavelen*0.750 ? 0x0000 : 0x3FF; break;
    }

    val *= ch1.volume;
    sound.buf[cursor + 0] = ch1.so1_enabled ? val : 0x0000;
    sound.buf[cursor + 1] = ch1.so2_enabled ? val : 0x0000;
    c++;

    sound.buf_end++;
    sound.buf_end %= sound.buf_size;
    //fprintf(stderr, "MIXED %i -> %i\n", (int)sound.buf_start, (int)sound.buf_end);
}

static void mix() {
    SDL_mutexP(mutex);
    ch1_mix();
    SDL_mutexV(mutex);

    mixed++;
}

static u16 get_available_samples() {
    u16 r;
    SDL_mutexP(mutex);
    if(sound.buf_start > sound.buf_end) {
        r = sound.buf_size - (sound.buf_start - sound.buf_end) + 1;
    }
    else {
        r = sound.buf_end - sound.buf_start;
    }
    SDL_mutexV(mutex);

    return r;
}

void move_buf(void *nichtVerwendet, Uint8 *stream, int length)
{

    //fprintf(stderr, "Moving %i bytes\n", length);

    u16 requested_samples = length / (sound.sample_size * 2);
    u16 available_samples;
    u16 served_samples;

    while((available_samples = get_available_samples()) < requested_samples) {
        SDL_Delay(2);
    }

    SDL_mutexP(mutex);
    if(sound.buf_start > sound.buf_end) {
        served_samples = sound.buf_size - sound.buf_start;
        if(served_samples >= requested_samples) {
            //fprintf(stderr, "1\n");
            memcpy(stream, &sound.buf[sound.buf_start], requested_samples * sound.sample_size * 2);
        }
        else {
            //fprintf(stderr, "2\n");
            u16 bytes = served_samples * sound.sample_size * 2;
            memcpy(&stream[0], &sound.buf[sound.buf_start], bytes);
            memcpy(&stream[bytes], &sound.buf[0], (requested_samples - served_samples) * sound.sample_size * 2);
        }
    }
    else {
        //fprintf(stderr, "3\n");
        memcpy(stream, &sound.buf[sound.buf_start], requested_samples * sound.sample_size * 2);
    }

    sound.buf_start += requested_samples;
    sound.buf_start %= sound.buf_size;
    SDL_mutexV(mutex);
    //fprintf(stderr, "MOVED %i -> %i\n", (int)sound.buf_start, (int)sound.buf_end);
    moved += requested_samples;

}


void sound_init() {
    SDL_AudioSpec format;


    mutex = SDL_CreateMutex();

    sound.freq = 44100;
    sound.buf_size = 4096;
    sound.buf_start = 0;
    sound.buf_end = 0;
    sound.sample_size = 2;
    sound.buf = malloc(sound.buf_size * sound.sample_size * 2);
    sound.last_timer_step = 0;
    sound.next_sample_cc = 0;

    /* Format: 16 Bit, stereo, 22 KHz */
    format.freq = sound.freq;
    format.format = AUDIO_U16;
    format.channels = 2;
    format.samples = 512;
    format.callback = move_buf;
    format.userdata = NULL;

    if ( SDL_OpenAudio(&format, NULL) < 0 ) {
        fprintf(stderr, "Audio-Gerät konnte nicht geöffnet werden: %s\n", SDL_GetError());
        exit(1);
    }
    SDL_PauseAudio(0);
}

void sound_close() {

}

void sound_reset() {
    sound.enabled = 1;
    sound.so1_volume = 0;
    sound.so2_volume = 0;
    sound.last_timer_step = 0;
    sound.next_sample = 0;
    sound.next_sample_cc = 0;

    ch1.duty = 0x00;
    ch1.freq = 0x00;
    ch1.volume = 0x0F;
    ch1.enabled = 0x01;
    ch1.so1_enabled = 0;
    ch1.so2_enabled = 0;

    env1.period = 0;
    env1.mode = 0;

    sweep.period = 0x00;
    sweep.negate = 0x00;
    sweep.shift = 0x00;
    sweep.shadow = 0x00;
    sweep.enable = 0x00;
}

void sound_step() {
    static time_t last_out = 0;
    if(last_out == 0) {
        last_out = SDL_GetTicks();
    }
    if(SDL_GetTicks() - last_out > 1000) {
        fprintf(stderr, "Moved %i | Mixed %i\n", moved, mixed);
        moved -= 44100;
        mixed -= 44100;
        last_out += 1000;
    }

   // timer_step();
    if(cpu.cc >= sound.next_sample_cc) {
        mix();
        sound.next_sample++;
        sound.next_sample_cc = ((uint64_t)cpu.freq*(uint64_t)sound.next_sample)/(uint64_t)sound.freq;
        //printf("Do %i %i\n", cpu.cc, sound.next_sample_cc);
    }
    else {
        //printf("nit\n");
    }

}

void sound_write_nr10(u8 val) {
    sweep.period = (val & 0x70) >> 4;
    sweep.negate = (val & 0x08) >> 3;
    sweep.shift = val & 0x07;
}

void sound_write_nr11(u8 val) {
    ch1.duty = (val & 0xC0) >> 6;
}

void sound_write_nr12(u8 val) {
    ch1.volume = val & 0xF0 >> 4;
    env1.mode = val & 0x08 >> 3;
    env1.period = val & 0x07;
}

void sound_write_nr13(u8 val) {
    ch1.freq &= 0xFF00;
    ch1.freq |= val;
}

void sound_write_nr14(u8 val) {
    ch1.freq &= 0xF8FF;
    ch1.freq |= (val&0x07)<<8;

    if(val & 0x80) {
        ch1_trigger();
    }
}

void sound_write_nr50(u8 val) {
    sound.so1_volume = val & 0x07;
    sound.so2_volume = (val >> 4) & 0x07;
}

void sound_write_nr51(u8 val) {
    ch1.so1_enabled = val & 0x01;
    ch1.so2_enabled = val & 0x10;
}

void sound_write_nr52(u8 val) {
    sound.enabled = val & 0x80;
}
