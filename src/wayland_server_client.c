#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "wayland_server_client.h"

struct _WaylandServerClient {
  int fd;
};

#include <stdio.h>
static void read_cb(void *user_data) {
  WaylandServerClient *self = user_data;

  uint8_t data[1024];
  ssize_t data_length = read(self->fd, data, 1024);
  if (data_length == -1) {
    return;
  }

  printf("%zi\n", data_length);
}

WaylandServerClient *wayland_server_client_new(MainLoop *loop, int fd) {
  WaylandServerClient *self = malloc(sizeof(WaylandServerClient));
  self->fd = fd;
  main_loop_add_fd(loop, fd, read_cb, self);
  return self;
}

WaylandServerClient *wayland_server_client_ref(WaylandServerClient *self) {
  // FIXME
  return self;
}

void wayland_server_client_unref(WaylandServerClient *self) {
  // FIXME
}
