#include <stdio.h>
#include <stdlib.h>

#include "wayland_server.h"

#include "socket_server.h"
#include "wayland_server_client.h"

struct _WaylandServer {
  MainLoop *loop;
  SocketServer *socket;
};

static void connect_cb(int fd, void *user_data) {
  WaylandServer *self = user_data;

  WaylandServerClient *client = wayland_server_client_new(self->loop, fd);
}

WaylandServer *wayland_server_new(MainLoop *loop) {
  WaylandServer *self = malloc(sizeof(WaylandServer));
  self->loop = main_loop_ref(loop);
  self->socket = socket_server_new(loop, connect_cb, self, NULL);
  return self;
}

WaylandServer *wayland_server_ref(WaylandServer *self) {
  // FIXME
  return self;
}

void wayland_server_unref(WaylandServer *self) {
  // FIXME
}

bool wayland_server_run(WaylandServer *self, const char *display) {
  const char *runtime_dir = getenv("XDG_RUNTIME_DIR");
  char path[1024];
  snprintf(path, 1024, "%s/%s", runtime_dir, display);

  return socket_server_run(self->socket, path);
}
