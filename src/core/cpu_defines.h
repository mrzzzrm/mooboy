#ifndef CPU_DEFINES_H
#define CPU_DEFINES_H

    #define A (cpu.af.b[0])
    #define F (cpu.af.b[1])
    #define B (cpu.bc.b[0])
    #define C (cpu.bc.b[1])
    #define D (cpu.de.b[0])
    #define E (cpu.de.b[1])
    #define H (cpu.hl.b[0])
    #define L (cpu.hl.b[1])

    #define AF (cpu.af.w)
    #define BC (cpu.bc.w)
    #define DE (cpu.de.w)
    #define HL (cpu.hl.w)
    #define SP (cpu.sp.w)
    #define PC (cpu.pc.w)

    #define BFETCH (mem_readb(PC++))
    #define WFETCH (((u16)mem_readb(PC++)) & ((u16)mem_readb(PC++)<<8))

    #define BC_RREF (mem_readb(BC))
    #define DE_RREF (mem_readb(DE))
    #define HL_RREF (mem_readb(HL))
    #define MEM_RREF(s) (mem_readb((s)));
    #define BC_WREF(s) (mem_writeb(BC_RREF, (s)))
    #define DE_WREF(s) (mem_writeb(DE_RREF, (s)))
    #define HL_WREF(s) (mem_writeb(HL_RREF, (s)))
    #define MEM_WREF(d, s) (mem_writeb((d), (s)));

#endif // CPU_DEFINES_H
