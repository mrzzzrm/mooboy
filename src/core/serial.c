#include "serial.h"
#include "cpu.h"
#include "moo.h"
#include "defines.h"
#include "sys/sys.h"
#include <string.h>
#include <stdio.h>

serial_t serial;

void serial_reset() {
    memset(&serial, 0x00, sizeof(serial));
    serial.external_period = 512;
}

void serial_step() {
//    u32 transfer_period;
//
//    transfer_period = serial.sc & 0x01 ? serial.internal_period : serial.external_period;
//
//    if(cpu.cc - serial.last_shift_cc >= transfer_period) {
//        serial.last_shift_cc += transfer_period;
//
//        if(serial.sc & 0x80 || sys_serial_incoming()) {
//            if(serial.sc & 0x80) {
//                serial.num_out++;
//                sys_serial_out_bit(serial.sb & 0x80 ? 1 : 0);
//
//                if(serial.num_out == 8) {
//                    serial.sc &= 0x7F;
//                    serial.num_out = 0;
//                    //cpu.irq |= IF_SERIAL;
//                    sys_serial_transfer_complete();
//                }
//            }
//
//            serial.sb <<= 1;
//
//            if(sys_serial_incoming()) {
//                serial.num_in++;
//                serial.sb |= sys_serial_in_bit();
//
//                if(serial.num_in == 8) {
//                    serial.num_in = 0;
//                    cpu.irq |= IF_SERIAL;
//                }
//            }
//            else {
//                serial.sb |= 0x01;
//            }
//        }
//    }
}


void serial_sc_write(u8 val) {
    //printf("SC = %.2X\n", val);

    if((serial.sc & 0x80) && !(val & 0x80)) {
        sys_serial_transfer_complete();
    }
    if((serial.sc & 0x02) != (val & 0x02)) {
        serial_update_internal_period();
    }

    serial.sc = val & 0x83;

    if(serial.sc & 0x80) {
       // printf("Starting serial transfer %.2X\n", serial.sb);
        serial.num_out = 0;
    }
}

void serial_update_internal_period() {
    if(moo.hw == DMG_HW) {
        serial.internal_period = 512;
    }
    else {
        if(serial.sc & 0x02) {
            serial.internal_period = 4;
        }
        else {
            serial.internal_period = 128;
        }
    }
    sys_serial_update_internal_period();
}


