#ifndef DEFINES_H
#define DEFINES_H

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

#define FETCH_BYTE (mem_read_byte(PC++))
#define FETCH_WORD (mem_read_word(PC++))

#define BREG(r) bregv[(r)]
#define WREG_SP(r) wregv_sp[(r)]
#define WREG_AF(r) wregv_af[(r)]

#define LN (c->op & 0x0F)
#define HN ((c->op & 0xF0) >> 4)

#define OPLD (c->opl.d)
#define OPRD (c->opr.d)
#define OPLB (*c->opl.b)
#define OPLW (*c->opl.w)
#define OPRB (*c->opr.b)
#define OPRW (*c->opr.w)

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

#endif
