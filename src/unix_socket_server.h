typedef struct _UnixSocketServer UnixSocketServer;

#pragma once

#include "fd.h"
#include "main_loop.h"
#include <stdbool.h>

typedef void (*UnixSocketServerConnectCallback)(UnixSocketServer *self, Fd *fd,
                                                void *user_data);

UnixSocketServer *
unix_socket_server_new(MainLoop *loop,
                       UnixSocketServerConnectCallback connect_callback,
                       void *user_data, void (*user_data_unref)(void *));

UnixSocketServer *unix_socket_server_ref(UnixSocketServer *self);

void unix_socket_server_unref(UnixSocketServer *self);

bool unix_socket_server_run(UnixSocketServer *self, const char *path);
