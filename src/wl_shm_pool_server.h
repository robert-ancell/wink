#pragma once

#include <stdint.h>

#include "wayland_payload_decoder.h"
#include "wayland_server_client.h"

typedef struct {
  void (*create_buffer)(uint32_t id, int32_t offset, int32_t width,
                        int32_t height, int32_t stride, uint32_t format,
                        void *user_data);
  void (*destroy)(void *user_data);
  void (*resize)(int32_t size, void *user_data);
} WlShmPoolServerRequestCallbacks;

typedef struct _WlShmPoolServer WlShmPoolServer;

WlShmPoolServer *
wl_shm_pool_server_new(WaylandServerClient *client, uint32_t id,
                       const WlShmPoolServerRequestCallbacks *request_callbacks,
                       void *user_data);

WlShmPoolServer *wl_shm_pool_server_ref(WlShmPoolServer *self);

void wl_shm_pool_server_unref(WlShmPoolServer *self);
