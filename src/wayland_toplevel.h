#pragma once

#include "wayland_client.h"

typedef struct _WaylandToplevel WaylandToplevel;

WaylandToplevel *wayland_toplevel_new(WaylandClient *client);

WaylandToplevel *wayland_toplevel_ref(WaylandToplevel *self);

void wayland_toplevel_unref(WaylandToplevel *self);
