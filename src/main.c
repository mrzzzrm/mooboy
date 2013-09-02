#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "core/moo.h"
#include "sys/sys.h"

static void close() {
    moo_close();
    sys_close();
}

static void init(int argc, const char **argv) {
    atexit(&close);

    sys_init(argc, argv);
    moo_init();
}

int main(int argc, const char **argv) {
    init(argc, argv);
    moo_run();
    close();

    return EXIT_SUCCESS;
}

