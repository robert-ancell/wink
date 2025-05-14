#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "socket_client.h"

#include "ref.h"

struct _SocketClient {
  ref_t ref;
  Fd *fd;
};

SocketClient *socket_client_new() {
  SocketClient *self = malloc(sizeof(SocketClient));
  ref_init(&self->ref);
  self->fd = NULL;
  return self;
}

SocketClient *socket_client_ref(SocketClient *self) {
  ref_inc(&self->ref);
  return self;
}

void socket_client_unref(SocketClient *self) {
  if (ref_dec(&self->ref)) {
    fd_unref(self->fd);
    free(self);
  }
}

bool socket_client_connect(SocketClient *self, const char *path) {
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

Fd *socket_client_get_fd(SocketClient *self) { return self->fd; }
