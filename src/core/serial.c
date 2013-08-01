#include "serial.h"

serial_t serial;

void serial_reset() {
    serial.sb = 0x00;
    serial.sc = 0x00;
    serial.external_period = 0;
    serial.last_bit_cc = 0;
    serial.bits_transfered = 0;
    serial.buf = 0;
}

void serial_step() {
    u32 transfer_period;

    if(serial.sc & 0x01) {
        transfer_period = serial.external_period;
    }
    else {
        if(emu.hw == DMG_HW) {
            transfer_period = 512;
        }
        else {
            if(serial.sc & 0x02) {
                transfer_period = 512;
            }
            else {
                transfer_period = 16;
            }
        }
    }

    if(cpu.cc - serial.last_bit >= transfer_period) {
        serial.last_bit_cc += transfer_period;
        sys_serial_shift(serial.sc & 0x80, serial.sb & 0x80)
        if(serial.sc & 0x80) {
            serial.bits_transfered++;

            if(serial.bits_transfered >= 8) {
                serial.sc &= 0x7F;
            }
        }
    }
}

void serial_start_transfer() {
    printf("Starting serial transfer\n");
    serial.bits_transfered = 0;
    serial.buf = 0x00;
}
