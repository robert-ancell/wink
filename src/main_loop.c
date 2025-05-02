#include "main_loop.h"

#include <poll.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct {
  MainLoopReadCallback read_callback;
  void *user_data;
} FdCallbacks;

struct _MainLoop {
  struct pollfd *fds;
  FdCallbacks *callbacks;
  nfds_t nfds;
};

MainLoop *main_loop_new() {
  MainLoop *self = malloc(sizeof(MainLoop));
  self->fds = NULL;
  self->nfds = 0;
  return self;
}

MainLoop *main_loop_ref(MainLoop *self) { return self; }

void main_loop_unref(MainLoop *self) {}

void main_loop_add_fd(MainLoop *self, int fd,
                      MainLoopReadCallback read_callback, void *user_data) {
  self->nfds++;
  self->fds = realloc(self->fds, sizeof(struct pollfd) * self->nfds);
  self->callbacks = realloc(self->callbacks, sizeof(FdCallbacks) * self->nfds);

  struct pollfd *f = &self->fds[self->nfds - 1];
  f->fd = fd;
  f->events = 0;
  if (read_callback != NULL) {
    f->events |= POLLIN;
  }
  f->revents = 0;

  FdCallbacks *c = &self->callbacks[self->nfds - 1];
  c->read_callback = read_callback;
  c->user_data = user_data;
}

void main_loop_run(MainLoop *self) {
  while (true) {
    poll(self->fds, self->nfds, 0);

    for (nfds_t i = 0; i < self->nfds; i++) {
      if (self->fds[i].revents & POLLIN) {
        self->callbacks[i].read_callback(self->callbacks[i].user_data);
      }
    }
  }
}
