#include "sys/sys.h"

#include <time.h>

#include "util/cmd.h"
#include "util/err.h"


void sys_init(int argc, const char** argv) {
    cmd_init(argc, argv);
}

void sys_close() {

}

bool sys_running()  {
    return true;
}

bool sys_new_rom()  {
    return true;
}

const char *sys_get_rompath() {
    const char *rom;
    if((rom = cmd_get("--rom")) == NULL) {
        err_set(ERR_ROM_NOT_FOUND);
        return NULL;
    }
    return rom;
}

void sys_sleep(time_t ticks) {
    usleep(ticks*1000);
}

void sys_error() {
    exit(EXIT_FAILURE);
}
