#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "socket_client.h"

#include "ref.h"

struct _SocketClient {
  ref_t ref;
  int fd;
};

SocketClient *socket_client_new() {
  SocketClient *self = malloc(sizeof(SocketClient));
  ref_init(&self->ref);
  self->fd = -1;
  return self;
}

SocketClient *socket_client_ref(SocketClient *self) {
  ref_inc(&self->ref);
  return self;
}

void socket_client_unref(SocketClient *self) {
  if (ref_dec(&self->ref)) {
    close(self->fd);
    free(self);
  }
}

bool socket_client_connect(SocketClient *self, const char *path) {
  self->fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (self->fd == -1) {
    return false;
  }

  struct sockaddr_un address;
  memset(&address, 0, sizeof(address));
  address.sun_family = AF_UNIX;
  snprintf(address.sun_path, sizeof(address.sun_path), "%s", path);
  if (connect(self->fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
    return false;
  }

  return true;
}

int socket_client_get_fd(SocketClient *self) { return self->fd; }
