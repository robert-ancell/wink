typedef struct _WaylandServer WaylandServer;

#pragma once

#include <stdbool.h>

#include "main_loop.h"
#include "wayland_server_client.h"

typedef void (*WaylandServerClientConnectedCallback)(
    WaylandServer *self, WaylandServerClient *client, void *user_data);

typedef void (*WaylandServerClientDisconnectedCallback)(
    WaylandServer *self, WaylandServerClient *client, void *user_data);

WaylandServer *wayland_server_new(
    MainLoop *loop, WaylandServerClientConnectedCallback connected_callback,
    WaylandServerClientDisconnectedCallback disconnected_callback,
    void *user_data, void (*user_data_unref)(void *));

WaylandServer *wayland_server_ref(WaylandServer *self);

void wayland_server_unref(WaylandServer *self);

bool wayland_server_run(WaylandServer *self, const char *display);
