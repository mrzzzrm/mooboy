#include "sound.h"
#include "cpu.h"
#include <SDL/SDL.h>

sound_t sound;
ch1_t ch1;
sweep_t sweep;

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
    if(ch1.env_period == 0) {
        return;
    }

    if(ch1.env_mode) {
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
    switch(step) {
        case 0: tick_length_counter();               break;
        case 2: tick_length_counter(); tick_sweep(); break;
        case 4: tick_length_counter();               break;
        case 6: tick_length_counter(); tick_sweep(); break;
        case 7: tick_envelope();                     break;
    }
}

static void ch1_mix() {

}

void more_audio(void *nichtVerwendet, Uint8 *_stream, int length)
{
    static size_t c = 0;
    unsigned int i, framesize, framecount;
    Uint16 *stream = (Uint16*)_stream;
    framesize = 2;
    int wavelen = 44100.0f / (double)(131072/(2048-ch1.freq));

    if(!sound.enabled ) {
        memset(_stream, 0x00, length);
        return;
    }



    for(i = 0; i < length/2; i += framesize) {
        u16 val;
        int in_wave = c%wavelen;

        switch(ch1.duty) {
            case 0x00: val = (float)in_wave <= (float)wavelen*0.125 ? 0x0000 : 0x3FF; break;
            case 0x01: val = (float)in_wave <= (float)wavelen*0.250 ? 0x0000 : 0x3FF; break;
            case 0x02: val = (float)in_wave <= (float)wavelen*0.500 ? 0x0000 : 0x3FF; break;
            case 0x03: val = (float)in_wave <= (float)wavelen*0.750 ? 0x0000 : 0x3FF; break;
        }

        val *= ch1.volume;
        stream[i+0] = ch1.so1_enabled ? val : 0x0000;
        stream[i+1] = ch1.so2_enabled ? val : 0x0000;
        c++;
    }
}


void sound_init() {
    SDL_AudioSpec format;

    /* Format: 16 Bit, stereo, 22 KHz */
    format.freq = 44100;
    format.format = AUDIO_U16;
    format.channels = 2;
    format.samples = 512;
    format.callback = more_audio;
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

    ch1.duty = 0x00;
    ch1.freq = 0x00;
    ch1.volume = 0x0F;
    ch1.env_mode = 0x00;
    ch1.env_period = 0x00;
    ch1.enabled = 0x01;
    ch1.so1_enabled = 0;
    ch1.so2_enabled = 0;

    sweep.period = 0x00;
    sweep.negate = 0x00;
    sweep.shift = 0x00;
    sweep.shadow = 0x00;
    sweep.enable = 0x00;
}

void sound_step() {
    timer_step();
    ch1_mix();
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
    ch1.env_mode = val & 0x08 >> 3;
    ch1.env_period = val & 0x07;
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
