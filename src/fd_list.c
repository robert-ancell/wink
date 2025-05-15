#include <stdlib.h>

#include "fd_list.h"

#include "ref.h"

struct _FdList {
  ref_t ref;
  Fd **fds;
  size_t fds_length;
};

FdList *fd_list_new(int fd) {
  FdList *self = malloc(sizeof(FdList));
  ref_init(&self->ref);
  self->fds = NULL;
  self->fds_length = 0;
  return self;
}

FdList *fd_list_ref(FdList *self) {
  ref_inc(&self->ref);
  return self;
}

void fd_list_unref(FdList *self) {
  if (ref_dec(&self->ref)) {
    for (size_t i = 0; i < self->fds_length; i++) {
      fd_unref(self->fds[i]);
    }
    free(self->fds);
    free(self);
  }
}

void fd_list_push(FdList *self, Fd *fd) {
  self->fds_length++;
  self->fds = realloc(self->fds, self->fds_length);
  self->fds[self->fds_length - 1] = fd_ref(fd);
}

Fd *fd_list_pop(FdList *self) {
  if (self->fds_length == 0) {
    return NULL;
  }

  Fd *fd = self->fds[0];
  for (size_t i = 1; i < self->fds_length; i++) {
    self->fds[i - 1] = self->fds[i];
  }
  self->fds_length--;
  self->fds = realloc(self->fds, self->fds_length);

  return fd;
}
