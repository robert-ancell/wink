#include "socket_server.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

struct _SocketServer {
  MainLoop *loop;
  int fd;
  SocketServerConnectCallback connect_callback;
  void *user_data;
  void (*user_data_unref)(void *);
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
                                void *user_data,
                                void (*user_data_unref)(void *)) {
  SocketServer *self = malloc(sizeof(SocketServer));
  self->loop = main_loop_ref(loop);
  self->fd = -1;
  self->connect_callback = connect_callback;
  self->user_data = user_data;
  self->user_data_unref = user_data_unref;
  return self;
}

SocketServer *socket_server_ref(SocketServer *self) {
  // FIXME
  return self;
}

void socket_server_unref(SocketServer *self) {
  // FIXME
}

bool socket_server_run(SocketServer *self, const char *path) {
  self->fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (self->fd == -1) {
    return false;
  }

  struct sockaddr_un address;
  memset(&address, 0, sizeof(address));
  address.sun_family = AF_UNIX;
  // int path_len =
  //     snprintf(address.sun_path + 1, sizeof(address.sun_path) - 1, "%s",
  //     path);
  // socklen_t address_length =
  //     offsetof(struct sockaddr_un, sun_path) + 1 + path_len;
  // if (bind(self->fd, (struct sockaddr *)&address, address_length) == -1) {
  //   return false;
  // }
  snprintf(address.sun_path, sizeof(address.sun_path), "%s", path);
  unlink(path);
  if (bind(self->fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
    return false;
  }

  main_loop_add_fd(self->loop, self->fd, read_cb, self, NULL);
  if (listen(self->fd, 1024) == -1) {
    return false;
  }

  return true;
}
