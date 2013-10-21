#ifndef CORE_DEFINES_H
#define CORE_DEFINES_H

#include <stdint.h>

typedef int8_t s8;
typedef int16_t s16;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define A (cpu.af.b[1])
#define F (cpu.af.b[0])
#define B (cpu.bc.b[1])
#define C (cpu.bc.b[0])
#define D (cpu.de.b[1])
#define E (cpu.de.b[0])
#define H (cpu.hl.b[1])
#define L (cpu.hl.b[0])

#define AF (cpu.af.w)
#define BC (cpu.bc.w)
#define DE (cpu.de.w)
#define HL (cpu.hl.w)
#define SP (cpu.sp.w)
#define PC (cpu.pc.w)

#define FZBIT 0x80
#define FNBIT 0x40
#define FHBIT 0x20
#define FCBIT 0x10

#define FZ (F&FZBIT)
#define FN (F&FNBIT)
#define FH (F&FHBIT)
#define FC (F&FCBIT)

#define FZZ(b) ((b) == 0 ? FZBIT : 0)
#define FCB0(b) (((b)&0x01)<<4)
#define FCB7(b) (((b)&0x80)>>3)

#define IF_VBLANK  0x01
#define IF_LCDSTAT 0x02
#define IF_TIMER   0x04
#define IF_SERIAL  0x08
#define IF_JOYPAD  0x10

#define IME_ON 3
#define IME_UP 2
#define IME_DOWN 1
#define IME_OFF 0

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))


#endif
