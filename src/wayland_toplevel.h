typedef struct _WaylandToplevel WaylandToplevel;

#pragma once

#include "wayland_client.h"

WaylandToplevel *wayland_toplevel_new(WaylandClient *client);

WaylandToplevel *wayland_toplevel_ref(WaylandToplevel *self);

void wayland_toplevel_unref(WaylandToplevel *self);

void wayland_toplevel_destroy(WaylandToplevel *self);

void wayland_toplevel_set_title(WaylandToplevel *self, const char *title);

void wayland_toplevel_set_app_id(WaylandToplevel *self, const char *app_id);
