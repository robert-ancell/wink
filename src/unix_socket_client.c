#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "unix_socket_client.h"

#include "ref.h"

struct _UnixSocketClient {
  ref_t ref;
  Fd *fd;
};

UnixSocketClient *unix_socket_client_new() {
  UnixSocketClient *self = malloc(sizeof(UnixSocketClient));
  ref_init(&self->ref);
  self->fd = NULL;
  return self;
}

UnixSocketClient *unix_socket_client_ref(UnixSocketClient *self) {
  ref_inc(&self->ref);
  return self;
}

void unix_socket_client_unref(UnixSocketClient *self) {
  if (ref_dec(&self->ref)) {
    fd_unref(self->fd);
    free(self);
  }
}

bool unix_socket_client_connect(UnixSocketClient *self, const char *path) {
  int fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd == -1) {
    return false;
  }
  self->fd = fd_new(fd);

  struct sockaddr_un address;
  memset(&address, 0, sizeof(address));
  address.sun_family = AF_UNIX;
  snprintf(address.sun_path, sizeof(address.sun_path), "%s", path);
  if (connect(fd_get(self->fd), (struct sockaddr *)&address, sizeof(address)) ==
      -1) {
    return false;
  }

  return true;
}

Fd *unix_socket_client_get_fd(UnixSocketClient *self) { return self->fd; }
