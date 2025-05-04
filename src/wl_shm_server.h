#pragma once

#include <stdint.h>

#include "wayland_server_client.h"

typedef struct {
  void (*create_pool)(void *user_data);
  void (*release)(void *user_data);
} WlShmServerRequestCallbacks;

typedef struct _WlShmServer WlShmServer;

WlShmServer *
wl_shm_server_new(WaylandServerClient *client, uint32_t id,
                  const WlShmServerRequestCallbacks *request_callbacks,
                  void *user_data);

WlShmServer *wl_shm_server_ref(WlShmServer *self);

void wl_shm_server_unref(WlShmServer *self);
