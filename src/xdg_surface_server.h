#pragma once

#include <stdint.h>

#include "wayland_payload_decoder.h"
#include "wayland_server_client.h"

typedef struct {
  void (*destroy)(void *user_data);
  void (*get_toplevel)(uint32_t id, void *user_data);
  void (*get_popup)(uint32_t id, uint32_t parent, uint32_t positioner,
                    void *user_data);
  void (*set_window_geometry)(int32_t x, int32_t y, int32_t width,
                              int32_t height, void *user_data);
  void (*ack_configure)(uint32_t serial, void *user_data);
} XdgSurfaceServerRequestCallbacks;

typedef struct _XdgSurfaceServer XdgSurfaceServer;

XdgSurfaceServer *xdg_surface_server_new(
    WaylandServerClient *client, uint32_t id,
    const XdgSurfaceServerRequestCallbacks *request_callbacks, void *user_data);

XdgSurfaceServer *xdg_surface_server_ref(XdgSurfaceServer *self);

void xdg_surface_server_unref(XdgSurfaceServer *self);

void xdg_surface_server_configure(XdgSurfaceServer *self, uint32_t serial);
