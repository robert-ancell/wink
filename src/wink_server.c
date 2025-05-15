#include <stdlib.h>

#include "wink_server.h"

#include "ref.h"
#include "wayland_client.h"
#include "wayland_server.h"
#include "wayland_toplevel.h"

struct _WinkServer {
  ref_t ref;
  WaylandClient *client;
  WaylandServer *server;
  WaylandToplevel *toplevel;
};

static void connected_cb(WaylandClient *client, void *user_data) {
  WinkServer *self = user_data;
  self->toplevel = wayland_toplevel_new(self->client);
  wayland_toplevel_set_title(self->toplevel, "Wink");
}

static void disconnected_cb(WaylandClient *client, void *user_data) {
  // WinkServer *self = user_data;
  // FIXME
}

#include <stdio.h>
static void client_connected_cb(WaylandServer *server,
                                WaylandServerClient *client, void *user_data) {
  // WinkServer *self = user_data;
  printf("connect\n");
}

static void client_disconnected_cb(WaylandServer *server,
                                   WaylandServerClient *client,
                                   void *user_data) {
  // WinkServer *self = user_data;
  printf("disconnect\n");
}

WinkServer *wink_server_new(MainLoop *loop) {
  WinkServer *self = malloc(sizeof(WinkServer));
  ref_init(&self->ref);
  self->client =
      wayland_client_new(loop, connected_cb, disconnected_cb, self, NULL);
  self->server = wayland_server_new(loop, client_connected_cb,
                                    client_disconnected_cb, self, NULL);
  return self;
}

WinkServer *wink_server_ref(WinkServer *self) {
  ref_inc(&self->ref);
  return self;
}

void wink_server_unref(WinkServer *self) {
  if (ref_dec(&self->ref)) {
    wayland_client_unref(self->client);
    wayland_server_unref(self->server);
    if (self->toplevel != NULL) {
      wayland_toplevel_unref(self->toplevel);
    }
    free(self);
  }
}

bool wink_server_run(WinkServer *self) {
  wayland_client_connect(self->client, NULL);
  if (!wayland_server_run(self->server, "wayland-99")) {
    return false;
  }

  return true;
}
