#include "err.h"

#include <stdlib.h>
#include <assert.h>

static int __errno;

static const char *msg[] = {
  "No Error set",
  "ROM not found or no ROM selected",
  "ROM is corrupted",
  "Illegal read access",
  "Illegal write access",
  "Legal, but yet unsupported operation"
};

void err_set(int new_errno) {
  assert(new_errno < sizeof(msg));
  __errno = new_errno;
}

void err_clear() {
    __errno = 0;
}

const char *err_msg() {
    return msg[__errno];
}

