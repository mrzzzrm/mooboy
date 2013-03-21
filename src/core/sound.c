#include "sound.h"
#include <SDL/SDL.h>

sound_t sound;
sound_channel_1_t sound_channel_1;

void more_audio(void *nichtVerwendet, Uint8 *_stream, int length)
{
    static size_t c = 0;
    unsigned int i, framesize, framecount;
    Uint16 *stream = (Uint16*)_stream;
    framesize = 2;
    int wavelen = 44100.0f / (double)(131072/(2048-sound_channel_1.frequency));

    if(!sound.enabled) {
        memset(_stream, 0x00, length);
        return;
    }

    for(i = 0; i < length/2; i += framesize) {
        u16 val;
        int in_wave = c%wavelen;

        switch(sound_channel_1.duty) {
            case 0x00: val = (float)in_wave <= (float)wavelen*0.125 ? 0x0000 : 0xFFFF; break;
            case 0x01: val = (float)in_wave <= (float)wavelen*0.250 ? 0x0000 : 0xFFFF; break;
            case 0x02: val = (float) in_wave <= (float)wavelen*0.500 ? 0x0000 : 0xFFFF; break;
            case 0x03: val = (float)in_wave <= (float)wavelen*0.750 ? 0x0000 : 0xFFFF; break;
        }

        stream[i+0] = sound_channel_1.so1_enabled ? val : 0x0000;
        stream[i+1] = sound_channel_1.so2_enabled ? val : 0x0000;
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

    sound_channel_1.duty = 0x00;
    sound_channel_1.frequency = 0x00;
    sound_channel_1.so1_enabled = 0;
    sound_channel_1.so2_enabled = 0;
}

void sound_step() {


}

void sound_write_nr10(u8 val) {

}

void sound_write_nr11(u8 val) {
    sound_channel_1.duty = (val & 0xC0) >> 6;
}

void sound_write_nr12(u8 val) {

}

void sound_write_nr13(u8 val) {
    sound_channel_1.frequency &= 0xFF00;
    sound_channel_1.frequency |= val;
}

void sound_write_nr14(u8 val) {
    sound_channel_1.frequency &= 0xF8FF;
    sound_channel_1.frequency |= (val&0x07)<<8;
}

void sound_write_nr50(u8 val) {
    sound.so1_volume = val & 0x07;
    sound.so2_volume = (val >> 4) & 0x07;
}

void sound_write_nr51(u8 val) {
    sound_channel_1.so1_enabled = val & 0x01;
    sound_channel_1.so2_enabled = val & 0x10;
}

void sound_write_nr52(u8 val) {
    sound.enabled = val & 0x80;
}
