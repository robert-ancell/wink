#include <poll.h>
#include <stdbool.h>
#include <stdlib.h>

#include "main_loop.h"

#include "ref.h"

typedef struct {
  MainLoopReadCallback read_callback;
  void *user_data;
  void (*user_data_unref)(void *);
} FdCallbacks;

struct _MainLoop {
  ref_t ref;
  struct pollfd *fds;
  size_t fds_length;
  FdCallbacks *callbacks;
};

MainLoop *main_loop_new() {
  MainLoop *self = malloc(sizeof(MainLoop));
  ref_init(&self->ref);
  self->fds = NULL;
  self->fds_length = 0;
  self->callbacks = NULL;
  return self;
}

MainLoop *main_loop_ref(MainLoop *self) {
  ref_inc(&self->ref);
  return self;
}

void main_loop_unref(MainLoop *self) {
  if (ref_dec(&self->ref)) {
    free(self->fds);
    free(self->callbacks);
    free(self);
  }
}

void main_loop_add_fd(MainLoop *self, Fd *fd,
                      MainLoopReadCallback read_callback, void *user_data,
                      void (*user_data_unref)(void *)) {
  self->fds_length++;
  self->fds = realloc(self->fds, sizeof(struct pollfd) * self->fds_length);
  self->callbacks =
      realloc(self->callbacks, sizeof(FdCallbacks) * self->fds_length);

  struct pollfd *f = &self->fds[self->fds_length - 1];
  f->fd = fd_get(fd);
  f->events = 0;
  if (read_callback != NULL) {
    f->events |= POLLIN;
  }
  f->revents = 0;

  FdCallbacks *c = &self->callbacks[self->fds_length - 1];
  c->read_callback = read_callback;
  c->user_data = user_data;
  c->user_data_unref = user_data_unref;
}

void main_loop_run(MainLoop *self) {
  while (true) {
    poll(self->fds, self->fds_length, 0);

    for (size_t i = 0; i < self->fds_length; i++) {
      if (self->fds[i].revents & POLLIN) {
        self->callbacks[i].read_callback(self->callbacks[i].user_data);
      }
    }
  }
}
