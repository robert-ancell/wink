#include <stdio.h>
#include <stdlib.h>

#include "wayland_server.h"

#include "ref.h"
#include "unix_socket_server.h"
#include "wayland_server_client.h"

struct _WaylandServer {
  ref_t ref;
  MainLoop *loop;
  WaylandServerClientConnectedCallback connected_callback;
  WaylandServerClientDisconnectedCallback disconnected_callback;
  void *user_data;
  void (*user_data_unref)(void *);
  UnixSocketServer *socket;
  WaylandServerClient **clients;
  size_t clients_length;
};

typedef struct {
  WaylandServer *self;
  void *user_data;
  void (*user_data_unref)(void *);
} ClientData;

static void disconnect_cb(WaylandServerClient *client, void *user_data) {
  WaylandServer *self = user_data;

  for (size_t i = 0; i < self->clients_length; i++) {
    if (self->clients[i] == client) {
      self->clients[i] = self->clients[self->clients_length - 1];
      self->clients_length--;

      self->disconnected_callback(self, client, self->user_data);

      wayland_server_client_unref(client);
      return;
    }
  }
}

static void connect_cb(UnixSocketServer *server, Fd *fd, void *user_data) {
  WaylandServer *self = user_data;

  WaylandServerClient *client =
      wayland_server_client_new(self->loop, fd, disconnect_cb, self, NULL);
  self->clients_length++;
  self->clients = realloc(self->clients,
                          sizeof(WaylandServerClient *) * self->clients_length);
  self->clients[self->clients_length - 1] = client;
  self->connected_callback(self, client, self->user_data);
}

WaylandServer *wayland_server_new(
    MainLoop *loop, WaylandServerClientConnectedCallback connected_callback,
    WaylandServerClientDisconnectedCallback disconnected_callback,
    void *user_data, void (*user_data_unref)(void *)) {
  WaylandServer *self = malloc(sizeof(WaylandServer));
  ref_init(&self->ref);
  self->loop = main_loop_ref(loop);
  self->connected_callback = connected_callback;
  self->disconnected_callback = disconnected_callback;
  self->user_data = user_data;
  self->user_data_unref = user_data_unref;
  self->socket = unix_socket_server_new(loop, connect_cb, self, NULL);
  return self;
}

WaylandServer *wayland_server_ref(WaylandServer *self) {
  ref_inc(&self->ref);
  return self;
}

void wayland_server_unref(WaylandServer *self) {
  if (ref_dec(&self->ref)) {
    main_loop_unref(self->loop);
    if (self->user_data_unref) {
      self->user_data_unref(self->user_data);
    }
    unix_socket_server_unref(self->socket);
    for (size_t i = 0; i < self->clients_length; i++) {
      wayland_server_client_unref(self->clients[i]);
    }
    free(self->clients);
    free(self);
  }
}

bool wayland_server_run(WaylandServer *self, const char *display) {
  const char *runtime_dir = getenv("XDG_RUNTIME_DIR");
  char path[1024];
  snprintf(path, 1024, "%s/%s", runtime_dir, display);

  return unix_socket_server_run(self->socket, path);
}
