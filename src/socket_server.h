#pragma once

#include "main_loop.h"
#include <stdbool.h>

typedef struct _SocketServer SocketServer;

typedef void (*SocketServerConnectCallback)(int fd, void *user_data);

SocketServer *socket_server_new(MainLoop *loop,
                                SocketServerConnectCallback connect_callback,
                                void *user_data,
                                void (*user_data_unref)(void *));

SocketServer *socket_server_ref(SocketServer *self);

void socket_server_unref(SocketServer *self);

bool socket_server_run(SocketServer *self, const char *path);
