#ifndef ASSERT_H
#define ASSERT_H

    #define assert_unsupported(expr, msg) if(!(expr)) {err_set(ERR_ROM_UNSUPPORTED); fprintf(stderr, "%s\n", (msg)); return 0; }
    #define assert_corrupt(expr, msg) if(!(expr)) {err_set(ERR_ROM_CORRUPT); fprintf(stderr, "%s\n", (msg)); return 0; }
    #define assert_illegal_read(expr, msg) if(!(expr)) {err_set(ERR_ILLEGAL_READ); fprintf(stderr, "%s\n", (msg)); return 0; }
    #define assert_illegal_write(expr, msg) if(!(expr)) {err_set(ERR_ILLEGAL_WRITE); fprintf(stderr, "%s\n", (msg)); return 0; }

#endif // ASSERT_H
