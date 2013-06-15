#include "int.h"
#include <stdio.h>
#include "debug.h"
#include "defines.h"

void int_init() {

}

void int_update() {

}

void int_before() {

}

void int_after() {

}

void int_cmd(const char *cmd) {

}

void int_exec(u8 flag) {
    debug_print_line_prefix();
    fprintf(stderr, "INT EXEC ");
    switch(flag) {
        case IF_VBLANK:  fprintf(stderr, "VBLANK INT"); break;
        case IF_LCDSTAT: fprintf(stderr, "LCDSTAT INT"); break;
        case IF_TIMER:   fprintf(stderr, "TIMER INT"); break;
        case IF_SERIAL:  fprintf(stderr, "SERIAL INT"); break;
        case IF_JOYPAD:  fprintf(stderr, "JOYPAD INT"); break;
    }
    fprintf(stderr, "\n");
}

void int_ie(u8 flag) {
    if(cpu.ie != flag) {
        debug_print_line_prefix();
        if((IF_VBLANK & cpu.ie) && !(IF_VBLANK & flag)) fprintf(stderr, "VBLANK INT disabled ");
        if(!(IF_VBLANK & cpu.ie) && (IF_VBLANK & flag)) fprintf(stderr, "VBLANK INT enabled ");
        if((IF_LCDSTAT & cpu.ie) && !(IF_LCDSTAT & flag)) fprintf(stderr, "LCDSTAT INT disabled ");
        if(!(IF_LCDSTAT & cpu.ie) && (IF_LCDSTAT & flag)) fprintf(stderr, "LCDSTAT INT enabled ");
        if((IF_TIMER & cpu.ie) && !(IF_TIMER & flag)) fprintf(stderr, "TIMER INT disabled ");
        if(!(IF_TIMER & cpu.ie) && (IF_TIMER & flag)) fprintf(stderr, "TIMER INT enabled ");
        if((IF_SERIAL & cpu.ie) && !(IF_SERIAL & flag)) fprintf(stderr, "SERIAL INT disabled ");
        if(!(IF_SERIAL & cpu.ie) && (IF_SERIAL & flag)) fprintf(stderr, "SERIAL INT enabled ");
        if((IF_JOYPAD & cpu.ie) && !(IF_JOYPAD & flag)) fprintf(stderr, "JOYPAD INT disabled ");
        if(!(IF_JOYPAD & cpu.ie) && (IF_JOYPAD & flag)) fprintf(stderr, "JOYPAD INT enabled ");
        fprintf(stderr, "\n");
    }
}

void int_ime(u8 flag) {
    if(flag != cpu.ime) {
        debug_print_line_prefix();
        if(flag && !cpu.ime)
            fprintf(stderr, "IME enabled\n");
        else
            fprintf(stderr, "IME disabled\n");
    }
}

void int_req(u8 flag) {
    if(cpu.irq != flag) {
        debug_print_line_prefix();
        if(!(IF_VBLANK & cpu.irq) && (IF_VBLANK & flag)) fprintf(stderr, "VBLANK INT requested ");
        if(!(IF_LCDSTAT & cpu.irq) && (IF_LCDSTAT & flag)) fprintf(stderr, "LCDSTAT INT requested ");
        if(!(IF_TIMER & cpu.irq) && (IF_TIMER & flag)) fprintf(stderr, "TIMER INT requested ");
        if(!(IF_SERIAL & cpu.irq) && (IF_SERIAL & flag)) fprintf(stderr, "SERIAL INT requested ");
        if(!(IF_JOYPAD & cpu.irq) && (IF_JOYPAD & flag)) fprintf(stderr, "JOYPAD INT requested ");
        fprintf(stderr, "\n");
    }
}

