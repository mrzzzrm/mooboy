#ifndef ERR_H
#define ERR_H

  #include "util/defines.h"

  #define ERR_NO_ERROR      0
  #define ERR_ROM_NOT_FOUND 1
  #define ERR_ROM_CORRUPT   2


  void err_set(int errno);
  void err_clear();

  const char *err_msg();

#endif // ERR_H
