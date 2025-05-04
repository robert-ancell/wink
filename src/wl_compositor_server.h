#pragma once

#include <stdint.h>

#include "wayland_server_client.h"

typedef struct {
  void (*create_surface)(uint32_t id, void *user_data);
  void (*create_region)(uint32_t id, void *user_data);
} WlCompositorServerRequestCallbacks;

typedef struct _WlCompositorServer WlCompositorServer;

WlCompositorServer *wl_compositor_server_new(
    WaylandServerClient *client, uint32_t id,
    const WlCompositorServerRequestCallbacks *request_callbacks,
    void *user_data);

WlCompositorServer *wl_compositor_server_ref(WlCompositorServer *self);

void wl_compositor_server_unref(WlCompositorServer *self);
