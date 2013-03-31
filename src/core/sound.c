#include "sound.h"
#include "cpu.h"
#include <stdlib.h>
#include <SDL/SDL.h>

typedef struct {
    u16 l, r;
} sample_t;

sound_t sound;
sqw_t ch1;
sqw_t ch2;
sweep_t sweep;
env_t env1, env2, env3;
wave_t wave;
noise_t noise;

static SDL_mutex *mutex;

static long moved = 0;
static long mixed = 0;

static void tick_length_counter(sqw_t *sqw) {
    if(sqw->length > 0) {
        sqw->length--;
        if(sqw->expires && sqw->length == 0) {
            sqw->on = 0;
        }
    }
}

static void tick_length_counters() {
    tick_length_counter(&ch1);
    tick_length_counter(&ch2);
    if(wave.length > 0) {
        wave.length--;
        if(wave.expires && wave.length == 0) {
            wave.on = 0;
        }
    }
    if(noise.length > 0) {
        noise.length--;
        if(noise.expires && noise.length == 0) {
            noise.on = 0;
        }
    }
}

static void tick_sweep() {
    if(sweep.period != 0) {
        sweep.tick++;
        if(sweep.tick >= sweep.period) {
            if(sweep.dir) {
                ch1.freq -= ch1.freq >> sweep.shift;
            }
            else {
                ch1.freq += ch1.freq >> sweep.shift;
            }
            sweep.tick = 0;
        }
    }
}

static void tick_envelope(env_t *env, u8 *volume) {
    if(env->sweep != 0) {
        env->tick++;
        if(env->tick >= env->sweep) {
            if(env->dir) {
                if(*volume < 0x0F) {
                    (*volume)++;
                }
            }
            else {
                if(*volume > 0x00) {
                    (*volume)--;
                }
            }
            env->tick = 0;
        }
    }
}

static void tick_envelopes() {
    tick_envelope(&env1, &ch1.volume);
    tick_envelope(&env2, &ch2.volume);
    tick_envelope(&env3, &noise.volume);
 }

static void timer_step() {
    u8 step = (cpu.cc >> 11) & 0x07;
    if(step != sound.last_timer_step) {
        switch(step) {
            case 0: tick_length_counters();               break;
            case 2: tick_length_counters(); tick_sweep(); break;
            case 4: tick_length_counters();               break;
            case 6: tick_length_counters(); tick_sweep(); break;
            case 7: tick_envelopes();                     break;
        }
        sound.last_timer_step = step;
    }
}

static sample_t sqw_mix(sqw_t *ch) {
    sample_t r = {0,0};
    int wavelen;
    int wavesam;
    u16 amp;

    if(!ch->on) {
        return r;
    }

    wavelen = sound.freq / (131072 / (2048 - ch->freq));
    wavesam = sound.sample % wavelen;

    switch(ch->duty) {
        case 0x00: amp = wavesam <= wavelen>>3 ? 0x0000 : 0x0100; break;
        case 0x01: amp = wavesam <= wavelen>>2 ? 0x0000 : 0x0100; break;
        case 0x02: amp = wavesam <= wavelen>>1 ? 0x0000 : 0x0100; break;
        case 0x03: amp = wavesam <= (wavelen>>2)*3 ? 0x0000 : 0x0100; break;
    }

    amp *= ch->volume;

    r.l = amp;
    r.r = amp;

    return r;
}

static sample_t wave_mix() {
    sample_t r = {0,0};
    u8 sample, mask;
    u16 amp;
    int wavelen;
    int wavesam;
    int realsam;

    if(!wave.on || wave.shift == 0) {
        return r;
    }

    wavelen = sound.freq / (65536/(2048-wave.freq));
    wavesam = (sound.sample % wavelen);
    realsam = (wavesam*0x20)/wavelen;

    if(realsam%2 == 0) {
        amp = wave.data[realsam>>1] >> 4;
    }
    else {
        amp = wave.data[realsam>>1] & 0x0F;
    }
    amp >>= (wave.shift-1);
    amp *= 0x100;

    r.l = amp;
    r.r = amp;
    return r;
}

static sample_t noise_mix() {
    sample_t r = {0,0};
    u32 freq;
    u16 amp;
    int wavelen;

    if(!noise.on || noise.volume == 0) {
        return r;
    }

    if(noise.divr == 0) {
        freq = (524288 * 2) >> (noise.shift+1);
    }
    else {
        freq = (524288 / noise.divr) >> (noise.shift+1);
    }
    wavelen = sound.freq / freq;
    if(wavelen == 0) {
        return r;
    }
    if(sound.sample / wavelen != (sound.sample+1) / wavelen) {
        u8 b = ((noise.lsfr+0x0001) & 0x03) >= 0x0002 ? 1 : 0;
        noise.lsfr >>= 1;
        noise.lsfr &= 0xBFFF;
        noise.lsfr |= b << 14;
        if(noise.width) {
            noise.lsfr &= 0xFFBF;
            noise.lsfr |= b << 6;
        }
    }
    if(noise.lsfr & 0x0001) {
        amp = 0x0000;
    }
    else {
        amp = 0x0100 * noise.volume;
    }


    r.l = amp;
    r.r = amp;
    return r;
}

void sound_init() {
    mutex = SDL_CreateMutex();

    sound.freq = 44100;
    sound.sample = 0;
    sound.buf_size = 4096;
    sound.buf_start = 0;
    sound.buf_end = 0;
    sound.sample_size = 2;
    sound.buf = malloc(sound.buf_size * sound.sample_size * 2);
    sound.last_timer_step = 0;
    sound.next_sample_cc = 0;
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

    memset(&ch1, 0x00, sizeof(ch1));
    memset(&env1, 0x00, sizeof(env1));
    memset(&sweep, 0x00, sizeof(sweep));
    memset(&ch2, 0x00, sizeof(ch2));
    memset(&env2, 0x00, sizeof(env2));
    memset(&wave, 0x00, sizeof(wave));
    memset(&noise, 0x00, sizeof(noise));

    noise.lsfr = 0xFFFF;
}

void sound_step() {
    static time_t last_out = 0;
    if(last_out == 0) {
        last_out = SDL_GetTicks();
    }
    if(SDL_GetTicks() - last_out > 1000) {
//      fprintf(stderr, "Moved %i | Mixed %i\n", moved, mixed);
        moved -= 44100;
        mixed -= 44100;
        last_out += 1000;
    }

    timer_step();
    if(cpu.cc >= sound.next_sample_cc) {
        sound_mix();
        sound.next_sample++;
        sound.next_sample_cc = ((uint64_t)cpu.freq*(uint64_t)sound.next_sample)/(uint64_t)sound.freq;
    }

}

void sound_lock() {
    SDL_mutexP(mutex);
}

void sound_unlock() {
    SDL_mutexV(mutex);
}

void sound_mix() {
    sample_t samples[4];
    u16 cursor;

    SDL_mutexP(mutex);
        u16 *buf = sound.buf;

        if(!sound.enabled) {
            buf[sound.buf_end*2 + 0] = 0x8000;
            buf[sound.buf_end*2 + 1] = 0x8000;
        }
        else {
            if((sound.buf_end + 1) % sound.buf_size == sound.buf_start) {
                printf("WARNING: Sound-Buffer overrun!\n");
                sound.buf_start = 0;
                sound.buf_end = 0;
            }

            samples[0] = sqw_mix(&ch1);
            samples[1] = sqw_mix(&ch2);
            samples[2] = wave_mix();
            samples[3] = noise_mix();

            buf[sound.buf_end*2 + 0] = (samples[0].l + samples[1].l + samples[2].l + samples[3].l)/4;
            buf[sound.buf_end*2 + 1] = (samples[0].r + samples[1].r + samples[2].r + samples[3].r)/4;
//            buf[sound.buf_end*2 + 0] = samples[3].l;
//            buf[sound.buf_end*2 + 1] = samples[3].r;
        }
        sound.buf_end++;
        sound.buf_end %= sound.buf_size;
        sound.sample++;
    SDL_mutexV(mutex);

//    mixed++;
}

void sound_write_nr10(u8 val) {
    sweep.period = (val & 0x70) >> 4;
    sweep.dir = (val & 0x08) >> 3;
    sweep.shift = val & 0x07;
}

void sound_write_nr11(u8 val) {
    ch1.duty = val >> 6;
    ch1.length = val & 0x3F;
}

void sound_write_nr12(u8 val) {
    ch1.volume = val >> 4;
    env1.dir = (val & 0x08) >> 3;
    env1.sweep = val & 0x07;
}

void sound_write_nr13(u8 val) {
    ch1.freq &= 0xFF00;
    ch1.freq |= val;
}

void sound_write_nr14(u8 val) {
    ch1.freq &= 0xF8FF;
    ch1.freq |= (val&0x07)<<8;
    ch1.expires = val & 0x40;
    if(val & 0x80) {
        ch1.on = 1;
    }
}

void sound_write_nr21(u8 val) {
    ch2.duty = val >> 6;
    ch2.length = 0x40 - (val & 0x3F);
}

void sound_write_nr22(u8 val) {
    ch2.volume = val >> 4;
    env2.dir = (val & 0x08) >> 3;
    env2.sweep = val & 0x07;
}

void sound_write_nr23(u8 val) {
    ch2.freq &= 0xFF00;
    ch2.freq |= val;
}

void sound_write_nr24(u8 val) {
    ch2.freq &= 0x00FF;
    ch2.freq |= (val&0x07) << 8;
    ch2.expires = val & 0x40;
    ch2.on |= val & 0x80;
}

void sound_write_nr30(u8 val) {
    wave.on = val;
}

void sound_write_nr31(u8 val) {
    wave.length = val;
}

void sound_write_nr32(u8 val) {
    wave.shift = (val & 0x60) >> 5;
}

void sound_write_nr33(u8 val) {
    wave.freq &= 0xFF00;
    wave.freq |= val;
}

void sound_write_nr34(u8 val) {
    wave.freq &= 0x00FF;
    wave.freq |= (val&0x07)<<8;
    wave.expires = val & 0x40;
    //wave.on |= val & 0x80;
}

void sound_write_wave(u8 i, u8 val) {
    wave.data[i] = val;
}

void sound_write_nr41(u8 val) {
    noise.length = 64-(val & 0x3F);
}

void sound_write_nr42(u8 val) {
    noise.volume = val >> 4;
    env3.dir = val & 0x08;
    env3.sweep = val & 0x07;
}

void sound_write_nr43(u8 val) {
    noise.shift = val >> 4;
    noise.width = val & 0x08;
    noise.divr = val & 0x07;
}

void sound_write_nr44(u8 val) {
    noise.expires = val & 0x40;
    if(val & 0x80) {
        noise.on = 1;
        noise.length = noise.length == 0 ? 0x40 : noise.length;
    }
}

void sound_write_nr50(u8 val) {
    sound.so1_volume = val & 0x07;
    sound.so2_volume = (val >> 4) & 0x07;
}

void sound_write_nr51(u8 val) {
//    ch1.left = val & 0x01;
//    ch1.right = val & 0x10;
}

void sound_write_nr52(u8 val) {
    sound.enabled = val & 0x80;
}
