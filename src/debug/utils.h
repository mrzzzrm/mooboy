#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

    const char *get_word(const char *str, char *buf, unsigned int bufsize);
    int get_bool(const char *str, const char *t, const char *f, const char **end);
    int begeq(const char *sstr, const char *lstr);
    int streq(const char *sstr, const char *lstr);

#endif
