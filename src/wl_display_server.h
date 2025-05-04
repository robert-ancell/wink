#pragma once

#include <stdint.h>

#include "wayland_payload_decoder.h"
#include "wayland_server_client.h"

typedef struct {
  void (*sync)(uint32_t callback, void *user_data);
  void (*get_registry)(uint32_t registry, void *user_data);
} WlDisplayServerRequestCallbacks;

typedef struct _WlDisplayServer WlDisplayServer;

WlDisplayServer *
wl_display_server_new(WaylandServerClient *client, uint32_t id,
                      const WlDisplayServerRequestCallbacks *request_callbacks,
                      void *user_data);

WlDisplayServer *wl_display_server_ref(WlDisplayServer *self);

void wl_display_server_unref(WlDisplayServer *self);

void wl_display_server_error(WlDisplayServer *self, uint32_t object_id,
                             uint32_t code, const char *message);

void wl_display_server_delete_id(WlDisplayServer *self, uint32_t id);
