#include "socket_server.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>

struct _SocketServer {
  MainLoop *loop;
  int fd;
  SocketServerConnectCallback connect_callback;
  void *user_data;
};

static void read_cb(void *user_data) {
  SocketServer *self = user_data;

  struct sockaddr_un address;
  socklen_t address_len = sizeof(address);
  int fd = accept(self->fd, (struct sockaddr *)&address, &address_len);
  if (fd == -1) {
    // FIXME
    return;
  }

  self->connect_callback(fd, self->user_data);
}

SocketServer *socket_server_new(MainLoop *loop,
                                SocketServerConnectCallback connect_callback,
                                void *user_data) {
  SocketServer *self = malloc(sizeof(SocketServer));
  self->loop = main_loop_ref(loop);
  self->fd = -1;
  self->connect_callback = connect_callback;
  self->user_data = user_data;
  return self;
}

bool socket_server_run(SocketServer *self) {
  self->fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (self->fd == -1) {
    return false;
  }

  struct sockaddr_un address;
  address.sun_family = AF_UNIX;
  snprintf(address.sun_path, sizeof(address.sun_path), "%s", "foo");
  if (bind(self->fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
    return false;
  }

  main_loop_add_fd(self->loop, self->fd, read_cb, self);
  if (listen(self->fd, 1024) == -1) {
    return false;
  }

  return true;
}
