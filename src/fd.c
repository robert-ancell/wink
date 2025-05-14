#include <stdlib.h>
#include <unistd.h>

#include "fd.h"

#include "ref.h"

struct _Fd {
  ref_t ref;
  int fd;
};

Fd *fd_new(int fd) {
  Fd *self = malloc(sizeof(Fd));
  ref_init(&self->ref);
  self->fd = fd;
  return self;
}

Fd *fd_ref(Fd *self) {
  ref_inc(&self->ref);
  return self;
}

void fd_unref(Fd *self) {
  if (ref_dec(&self->ref)) {
    if (self->fd >= 0) {
      close(self->fd);
    }
    free(self);
  }
}

int fd_get(Fd *self) { return self->fd; }

int fd_take(Fd *self) {
  int fd = self->fd;
  self->fd = -1;
  return fd;
}
