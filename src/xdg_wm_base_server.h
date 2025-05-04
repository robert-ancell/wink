#pragma once

#include <stdint.h>

#include "wayland_payload_decoder.h"
#include "wayland_server_client.h"

typedef struct {
  void (*destroy)(void *user_data);
  void (*create_positioner)(uint32_t id, void *user_data);
  void (*get_xdg_surface)(uint32_t id, uint32_t surface, void *user_data);
  void (*pong)(uint32_t serial, void *user_data);
} XdgWmBaseServerRequestCallbacks;

typedef struct _XdgWmBaseServer XdgWmBaseServer;

XdgWmBaseServer *
xdg_wm_base_server_new(WaylandServerClient *client, uint32_t id,
                       const XdgWmBaseServerRequestCallbacks *request_callbacks,
                       void *user_data);

XdgWmBaseServer *xdg_wm_base_server_ref(XdgWmBaseServer *self);

void xdg_wm_base_server_unref(XdgWmBaseServer *self);

void xdg_wm_base_server_ping(XdgWmBaseServer *self, uint32_t serial);
