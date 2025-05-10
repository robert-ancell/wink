#pragma once

#include <stdbool.h>

typedef struct _SocketClient SocketClient;

SocketClient *socket_client_new();

SocketClient *socket_client_ref(SocketClient *self);

void socket_client_unref(SocketClient *self);

bool socket_client_connect(SocketClient *self, const char *path);

int socket_client_get_fd(SocketClient *self);
