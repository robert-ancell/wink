#pragma once

#include "main_loop.h"
#include <stdbool.h>

typedef struct _WaylandServer WaylandServer;

WaylandServer *wayland_server_new(MainLoop *loop);

WaylandServer *wayland_server_ref(WaylandServer *self);

void wayland_server_unref(WaylandServer *self);

bool wayland_server_run(WaylandServer *self, const char *display);
