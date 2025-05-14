#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "unix_socket_server.h"

#include "ref.h"

struct _UnixSocketServer {
  ref_t ref;
  MainLoop *loop;
  Fd *fd;
  UnixSocketServerConnectCallback connect_callback;
  void *user_data;
  void (*user_data_unref)(void *);
};

static void read_cb(MainLoop *loop, void *user_data) {
  UnixSocketServer *self = user_data;

  struct sockaddr_un address;
  socklen_t address_len = sizeof(address);
  int fd = accept(fd_get(self->fd), (struct sockaddr *)&address, &address_len);
  if (fd == -1) {
    // FIXME: handle error
    return;
  }

  Fd *fd_object = fd_new(fd);
  self->connect_callback(self, fd_object, self->user_data);
  fd_unref(fd_object);
}

UnixSocketServer *
unix_socket_server_new(MainLoop *loop,
                       UnixSocketServerConnectCallback connect_callback,
                       void *user_data, void (*user_data_unref)(void *)) {
  UnixSocketServer *self = malloc(sizeof(UnixSocketServer));
  ref_init(&self->ref);
  self->loop = main_loop_ref(loop);
  self->fd = NULL;
  self->connect_callback = connect_callback;
  self->user_data = user_data;
  self->user_data_unref = user_data_unref;
  return self;
}

UnixSocketServer *unix_socket_server_ref(UnixSocketServer *self) {
  ref_inc(&self->ref);
  return self;
}

void unix_socket_server_unref(UnixSocketServer *self) {
  if (ref_dec(&self->ref)) {
    main_loop_unref(self->loop);
    fd_unref(self->fd);
    // FIXME: Delete socket, or is that done in shutdown?
    if (self->user_data_unref) {
      self->user_data_unref(self->user_data);
    }
    free(self);
  }
}

bool unix_socket_server_run(UnixSocketServer *self, const char *path) {
  int fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd == -1) {
    return false;
  }
  self->fd = fd_new(fd);

  struct sockaddr_un address;
  memset(&address, 0, sizeof(address));
  address.sun_family = AF_UNIX;
  snprintf(address.sun_path, sizeof(address.sun_path), "%s", path);
  unlink(path);
  if (bind(fd_get(self->fd), (struct sockaddr *)&address, sizeof(address)) ==
      -1) {
    return false;
  }

  main_loop_add_fd(self->loop, self->fd, read_cb, self, NULL);
  if (listen(fd_get(self->fd), 1024) == -1) {
    return false;
  }

  return true;
}
