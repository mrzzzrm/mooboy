#ifndef ERR_H
#define ERR_H

  #include "util/defines.h"

  #define ERR_NO_ERROR        0
  #define ERR_ROM_NOT_FOUND   1
  #define ERR_ROM_CORRUPT     2
  #define ERR_ILLEGAL_READ    3
  #define ERR_ILLEGAL_WRITE   4
  #define ERR_ROM_UNSUPPORTED 5


  void err_set(int new_errno);
  void err_clear();

  const char *err_msg();

#endif // ERR_H
