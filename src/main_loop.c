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
  FdCallbacks *callbacks;
  nfds_t nfds;
};

MainLoop *main_loop_new() {
  MainLoop *self = malloc(sizeof(MainLoop));
  ref_init(&self->ref);
  self->fds = NULL;
  self->callbacks = NULL;
  self->nfds = 0;
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
  }
}

void main_loop_add_fd(MainLoop *self, int fd,
                      MainLoopReadCallback read_callback, void *user_data,
                      void (*user_data_unref)(void *)) {
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
  c->user_data_unref = user_data_unref;
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
