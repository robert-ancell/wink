#pragma once

#include <stdint.h>

#include "wayland_payload_decoder.h"
#include "wayland_server_client.h"

typedef struct {
  void (*destroy)(void *user_data);
} WlBufferServerRequestCallbacks;

typedef struct _WlBufferServer WlBufferServer;

WlBufferServer *
wl_buffer_server_new(WaylandServerClient *client, uint32_t id,
                     const WlBufferServerRequestCallbacks *request_callbacks,
                     void *user_data);

WlBufferServer *wl_buffer_server_ref(WlBufferServer *self);

void wl_buffer_server_unref(WlBufferServer *self);

void wl_buffer_server_release(WlBufferServer *self);
