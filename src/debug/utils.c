#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>


const char *get_word(const char *str, char *buf, unsigned int bufsize) {
    unsigned int i;
    unsigned int str_size = strlen(str);

    for(;; str++) {
        if(*str == '\0') {
            buf[0] = '\0';
            return str;
        }
        if(isprint(*str) && !isspace(*str))
            break;
    }

    for(i = 0; i < str_size-1 && i < bufsize-1; i++) {
        if(isprint(str[i]) && !isspace(str[i]))
            buf[i] = str[i];
        else
            break;
    }
    buf[i] = '\0';
    return &str[i];
}

int get_bool(const char *str, const char *t, const char *f, const char **end) {
    char cmd[256];

    *end = get_word(str, cmd, sizeof(cmd));

    if(streq(t, cmd)) {
        return 1;
    }
    else if(streq(f, cmd)) {
        return 0;
    }
    else {
        return -1;
    }
}

int begeq(const char *sstr, const char *lstr) {
    if(strlen(lstr) < strlen(sstr))
        return 0;
    return memcmp(sstr, lstr, strlen(sstr)) == 0;
}

int streq(const char *sstr, const char *lstr) {
    return strcmp(sstr, lstr) == 0;
}
