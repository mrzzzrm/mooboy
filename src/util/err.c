#include "err.h"

#include <stdlib.h>
#include <assert.h>

static int errno;

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
  errno = new_errno;
}

void err_clear() {
    errno = 0;
}

const char *err_msg() {
    return msg[errno];
}

