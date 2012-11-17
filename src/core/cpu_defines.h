#ifndef CPU_DEFINES_H
#define CPU_DEFINES_H

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

    #define FETCHB (mem_readb(PC++))
    #define FETCHW (((u16)mem_readb(PC++)) & ((u16)mem_readb(PC++)<<8))

    #define REG(r) regv[(r)]

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
    #define FHC(b1, b2)
    #define FHB(b1, b2)
    #define FCC(b1, b2)
    #define FCB(b1, b2)
    #define FCB0(b) (((b)&0x1)<<4)
    #define FCB7(b) (((b)&0x80)>>3)




#endif // CPU_DEFINES_H
