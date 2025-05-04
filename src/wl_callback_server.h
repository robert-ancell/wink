#pragma once

#include <stdint.h>

#include "wayland_payload_decoder.h"
#include "wayland_server_client.h"

typedef struct {
} WlCallbackServerRequestCallbacks;

typedef struct _WlCallbackServer WlCallbackServer;

WlCallbackServer *wl_callback_server_new(
    WaylandServerClient *client, uint32_t id,
    const WlCallbackServerRequestCallbacks *request_callbacks, void *user_data);

WlCallbackServer *wl_callback_server_ref(WlCallbackServer *self);

void wl_callback_server_unref(WlCallbackServer *self);

void wl_callback_server_done(WlCallbackServer *self, uint32_t callback_data);
