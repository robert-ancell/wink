typedef struct _UnixSocketClient UnixSocketClient;

#pragma once

#include <stdbool.h>

#include "fd.h"

UnixSocketClient *unix_socket_client_new();

UnixSocketClient *unix_socket_client_ref(UnixSocketClient *self);

void unix_socket_client_unref(UnixSocketClient *self);

bool unix_socket_client_connect(UnixSocketClient *self, const char *path);

Fd *unix_socket_client_get_fd(UnixSocketClient *self);
