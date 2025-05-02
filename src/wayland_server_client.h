#pragma once

#include "main_loop.h"
#include <stdbool.h>

typedef struct _WaylandServerClient WaylandServerClient;

WaylandServerClient *wayland_server_client_new(MainLoop *loop, int fd);

WaylandServerClient *wayland_server_client_ref(WaylandServerClient *self);

void wayland_server_client_unref(WaylandServerClient *self);
