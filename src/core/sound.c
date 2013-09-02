#include "sound.h"
#include "cpu.h"
#include "defines.h"
#include "sys/sys.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    u16 l, r;
} sample_t;

sound_t sound;
sqw_t sqw[2];
sweep_t sweep;
env_t env[3];
wave_t wave;
noise_t noise;


static void tick_length_counter(counter_t *counter, u8 *on) {
    if(counter->length > 0) {
        counter->length--;
        if(counter->expires && counter->length == 0) {
            *on = 0;
        }
    }
}

static void tick_length_counters() {
    tick_length_counter(&sqw[0].counter, &sqw[0].on);
    tick_length_counter(&sqw[1].counter, &sqw[1].on);
    tick_length_counter(&wave.counter, &wave.on);
    tick_length_counter(&noise.counter, &noise.on);
}

static void tick_sweep() {
    if(sweep.period != 0) {
        sweep.tick++;
        if(sweep.tick >= sweep.period) {
            if(sweep.dir) {
                sqw[0].freq -= sqw[0].freq >> sweep.shift;
            }
            else {
                sqw[0].freq += sqw[0].freq >> sweep.shift;
            }
            sqw[0].freq &= 0x07FF;
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
    tick_envelope(&env[0], &sqw[0].volume);
    tick_envelope(&env[1], &sqw[1].volume);
    tick_envelope(&env[2], &noise.volume);
 }

static void timer_step() {
    u8 step = (sound.tick_cc >> 11) & 0x07;
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
    u16 amp = 0;

    if(!ch->on) {
        return r;
    }

    if(ch->freq == 2048) {
   //     printf("?");
        return r;
    }
    wavelen = sound.freq / (131072 / (2048 - ch->freq));
    if(wavelen == 0) {
        return r;
    }
    wavesam = sound.sample % wavelen;

    switch(ch->duty) {
        case 0x00: amp = wavesam <= wavelen>>3 ? 0x0 : 0x1; break;
        case 0x01: amp = wavesam <= wavelen>>2 ? 0x0 : 0x1; break;
        case 0x02: amp = wavesam <= wavelen>>1 ? 0x0 : 0x1; break;
        case 0x03: amp = wavesam <= (wavelen>>2)*3 ? 0x0 : 0x1; break;
    }

    amp *= ch->volume;

    r.l = ch->l ? amp : 0;
    r.r = ch->r ? amp : 0;
    return r;
}

static sample_t wave_mix() {
    sample_t r = {0,0};
    u16 amp;
    int wavelen;
    int wavesam;
    int realsam;

    if(!wave.on || wave.shift == 0) {
        return r;
    }

    assert(wave.freq != 2048);
    wavelen = sound.freq / (65536/(2048 - wave.freq));
    if(wavelen == 0) {
        return r;
    }
    else {
        wavesam = (sound.sample % wavelen);
        realsam = (wavesam*0x20)/wavelen;
    }


    if(realsam%2 == 0) {
        amp = wave.data[realsam>>1] >> 4;
    }
    else {
        amp = wave.data[realsam>>1] & 0x0F;
    }
    amp >>= (wave.shift-1);

    r.l = wave.l ? amp : 0;
    r.r = wave.r ? amp : 0;
    return r;
}

// TODO: There definitly is a bug in here...
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
    assert(freq != 0);
    wavelen = sound.freq / freq;

    if(wavelen == 0 || sound.sample / wavelen != (sound.sample+1) / wavelen) {
        u8 b = ((noise.lsfr+0x0001) & 0x03) >= 0x0002 ? 1 : 0;
        noise.lsfr >>= 1;
        noise.lsfr &= 0xBFFF;
        noise.lsfr |= b << 14;
        if(noise.width) {
            noise.lsfr &= 0xFFBF;
            noise.lsfr |= b << 6;
        }
    }

    amp = noise.lsfr & 0x0001 ? 0x0 : noise.volume;

    r.l = noise.l ? amp : 0;
    r.r = noise.r ? amp : 0;
    return r;
}

void sound_init() {
    memset(&sound, 0x00, sizeof(sound));

    sound.freq = sys.sound_freq;
    sound.sample = 0;
    sound.buf_size = 4096;
    sound.buf_start = 0;
    sound.buf_end = 0;
    sound.sample_size = 2;
    sound.buf = malloc(sound.buf_size * sound.sample_size * 2);
    sound.last_timer_step = 0;
    sound.next_sample_cc = 0;
    sound.counter = NORMAL_CPU_FREQ;
}

void sound_close() {

}

void sound_reset() {
    sound.on = 1;
    sound.so1_volume = 7;
    sound.so2_volume = 7;
    sound.last_timer_step = 0;
    sound.next_sample = 0;
    sound.next_sample_cc = 0;

    memset(&sqw, 0x00, sizeof(sqw));
    memset(&env, 0x00, sizeof(env));
    memset(&sweep, 0x00, sizeof(sweep));
    memset(&wave, 0x00, sizeof(wave));
    memset(&noise, 0x00, sizeof(noise));

    noise.lsfr = 0xFFFF;
}

void sound_step(int nfcs) {
    timer_step();

    sound.cc += nfcs;
    sound.tick_cc += nfcs;
    sound.tick_cc &= 0x3FFF;
    if(sound.cc >= sound.mix_threshold) {
        sound_mix();

        sound.cc -= sound.mix_threshold;
        sound.mix_threshold = sound.counter / sound.freq;
        sound.counter = NORMAL_CPU_FREQ + (sound.counter % sound.freq);
        sound.next_sample++;
    }
//    if(cpu.nfcc >= sound.next_sample_cc) {
//        sound_mix();
//        sound.next_sample++;
//        sound.next_sample_cc = (NORMAL_CPU_FREQ*(uint64_t)sound.next_sample)/(uint64_t)sound.freq;
//    }
}

// TODO: Handle userdefined on/off elsewhere, sound-off shouldn't use resources
void sound_mix() {
    sample_t samples[4];
    sys_lock_audiobuf();

    u16 *buf = (u16*)sound.buf;

    if(!sound.on) {
        buf[sound.buf_end*2 + 0] = 0x0000;
        buf[sound.buf_end*2 + 1] = 0x0000;
    }
    else {
        if((sound.buf_end + 1) % sound.buf_size == sound.buf_start) {
            //printf("WARNING: Sound-Buffer overrun!\n");
            sound.buf_start = 0;
            sound.buf_end = 0;
        }

        samples[0] = sqw_mix(&sqw[0]);
        samples[1] = sqw_mix(&sqw[1]);
        samples[2] = wave_mix();
        samples[3] = noise_mix();
        buf[sound.buf_end*2 + 0] = (samples[0].l + samples[1].l + samples[2].l + samples[3].l)*sound.so1_volume*0x40;
        buf[sound.buf_end*2 + 1] = (samples[0].r + samples[1].r + samples[2].r + samples[3].r)*sound.so2_volume*0x40;
//
//        buf[sound.buf_end*2 + 0] = (samples[0].l)*sound.so1_volume*0x40;
//        buf[sound.buf_end*2 + 1] = (samples[0].r)*sound.so2_volume*0x40;
    }
    sound.buf_end++;
    sound.buf_end %= sound.buf_size;
    sound.sample++;

    sys_unlock_audiobuf();
}

void sound_write(u8 sadr, u8 val) {
    switch(sadr) {
        case 0x10:
            sweep.period = (val & 0x70) >> 4;
            sweep.dir = (val & 0x08) >> 3;
            sweep.shift = val & 0x07;
        break;
        case 0x11:
            sqw[0].duty = val >> 6;
            sqw[0].counter.length = val & 0x3F;
        break;
        case 0x12:
            sqw[0].volume = val >> 4;
            env[0].dir = (val & 0x08) >> 3;
            env[0].sweep = val & 0x07;
        break;
        case 0x13:
            sqw[0].freq &= 0xFF00;
            sqw[0].freq |= val;
        break;
        case 0x14:
            sqw[0].freq &= 0xF8FF;
            sqw[0].freq |= (val&0x07)<<8;
            sqw[0].counter.expires = val & 0x40;
            if(val & 0x80) {
                sqw[0].on = 1;
            }
        break;
        case 0x16:
            sqw[1].duty = val >> 6;
            sqw[1].counter.length = 0x40 - (val & 0x3F);
        break;
        case 0x17:
            sqw[1].volume = val >> 4;
            env[1].dir = (val & 0x08) >> 3;
            env[1].sweep = val & 0x07;
        break;
        case 0x18:
            sqw[1].freq &= 0xFF00;
            sqw[1].freq |= val;
        break;
        case 0x19:
            sqw[1].freq &= 0x00FF;
            sqw[1].freq |= (val&0x07) << 8;
            sqw[1].counter.expires = val & 0x40;
            sqw[1].on |= val & 0x80;
        break;
        case 0x1A:
            wave.on = val;
        break;
        case 0x1B:
            wave.counter.length = val;
        break;
        case 0x1C:
            wave.shift = (val & 0x60) >> 5;
        break;
        case 0x1D:
            wave.freq &= 0xFF00;
            wave.freq |= val;
        break;
        case 0x1E:
            wave.freq &= 0x00FF;
            wave.freq |= (val&0x07)<<8;
            wave.counter.expires = val & 0x40;
            //wave.on |= val & 0x80;
        break;
        case 0x20:
            noise.counter.length = 64-(val & 0x3F);
        break;
        case 0x21:
            noise.volume = val >> 4;
            env[2].dir = val & 0x08;
            env[2].sweep = val & 0x07;
        break;
        case 0x22:
            noise.shift = val >> 4;
            noise.width = val & 0x08;
            noise.divr = val & 0x07;
        break;
        case 0x23:
            noise.counter.expires = val & 0x40;
            if(val & 0x80) {
                noise.on = 1;
                noise.counter.length = noise.counter.length == 0 ? 0x40 : noise.counter.length;
            }
        break;
        case 0x24:
            sound.so1_volume = val & 0x07;
            sound.so2_volume = val >> 4;
        break;
        case 0x25:
            sqw[0].l = val & 0x01; sqw[0].r = val & 0x10;
            sqw[1].l = val & 0x02; sqw[1].r = val & 0x20;
            wave.l =  val & 0x04; wave.r =  val & 0x40;
            noise.l = val & 0x08; noise.r = val & 0x80;
        break;
        case 0x26:
            sound.on = val & 0x80;
        break;

        case 0x30: case 0x31: case 0x32: case 0x33:
        case 0x34: case 0x35: case 0x36: case 0x37:
        case 0x38: case 0x39: case 0x3A: case 0x3B:
        case 0x3C: case 0x3D: case 0x3E: case 0x3F:
            wave.data[sadr - 0x30] = val;
        break;
    }
}

u8 sound_read(u8 sadr) {
    switch(sadr) {
        case 0x10:
            return (sweep.period << 4) | (sweep.dir << 3) | sweep.shift;
        break;
        case 0x11:
            return sqw[0].duty << 6;
        break;
        case 0x12:
            return (sqw[0].volume << 4) || (env[0].dir << 3) | env[0].sweep;
        break;
        case 0x13:
            return 0x00;
        break;
        case 0x14:
            return sqw[0].counter.expires;
        break;
        case 0x16:
            return sqw[1].duty << 6;
        break;
        case 0x17:
            return (sqw[1].volume << 4) | (env[1].dir << 3) | env[1].sweep;
        break;
        case 0x18:
            return 0x00;
        break;
        case 0x19:
            return sqw[1].counter.expires;
        break;
        case 0x1A:
            return wave.on;
        break;
        case 0x1B:
            return wave.counter.length;
        break;
        case 0x1C:
            return wave.shift << 5;
        break;
        case 0x1D:
            return 0x00;
        break;
        case 0x1E:
            return wave.counter.expires;
        break;
        case 0x20:
            return noise.counter.length;
        break;
        case 0x21:
            return (noise.volume << 4) | (env[2].dir << 3) | (env[2].sweep);
        break;
        case 0x22:
            return (noise.shift << 4) || (noise.width) | (noise.divr);
        break;
        case 0x23:
            return noise.counter.expires;
        break;
        case 0x24:
            return sound.so1_volume | (sound.so2_volume << 4);
        break;
        case 0x25:
            return
            (sqw[0].l ? 0x01 : 0x00) | (sqw[0].r ? 0x10 : 0x00) |
            (sqw[1].l ? 0x02 : 0x00) | (sqw[1].r ? 0x20 : 0x00) |
            (wave.l ? 0x04 : 0x00) | (wave.r ? 0x40 : 0x00) |
            (noise.l ? 0x08 : 0x00) | (noise.r ? 0x80 : 0x00);
        break;
        case 0x26:
            return
            (sound.on ? 0x80 : 0x00) |
            (noise.on ? 0x08 : 0x00) |
            (wave.on ? 0x04 : 0x00) |
            (sqw[1].on ? 0x02 : 0x00) |
            (sqw[0].on ? 0x01 : 0x00);
        break;

        case 0x30: case 0x31: case 0x32: case 0x33:
        case 0x34: case 0x35: case 0x36: case 0x37:
        case 0x38: case 0x39: case 0x3A: case 0x3B:
        case 0x3C: case 0x3D: case 0x3E: case 0x3F:
            return wave.data[sadr - 0x30];
        break;

        default:
            printf("%.4X: Unhandled sound read @ %.2X\n", PC-1, sadr);
    }

    return 0;
}

