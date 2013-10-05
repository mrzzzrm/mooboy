#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "core/moo.h"
#include "sys/sys.h"
#include "menu/menu.h"

static void close() {
    printf("-- Closing MooBoy --\n");
    moo_close();
    menu_close();
    sys_close();
}

static void init(int argc, const char **argv) {
    printf("-- Starting MooBoy --\n");
    atexit(&close);

    sys_init(argc, argv);
    menu_init();
    moo_init();
}

int main(int argc, const char **argv) {
    init(argc, argv);
    moo_main();

    return EXIT_SUCCESS;
}

