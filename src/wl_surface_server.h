#pragma once

#include <stdint.h>

#include "wayland_payload_decoder.h"
#include "wayland_server_client.h"

typedef struct {
  void (*destroy)(void *user_data);
  void (*attach)(uint32_t buffer, int32_t x, int32_t y, void *user_data);
  void (*damage)(int32_t x, int32_t y, int32_t width, int32_t height,
                 void *user_data);
  void (*frame)(uint32_t callback, void *user_data);
  void (*set_opaque_region)(uint32_t region, void *user_data);
  void (*set_input_region)(uint32_t region, void *user_data);
  void (*commit)(void *user_data);
  void (*set_buffer_transform)(int32_t transform, void *user_data);
  void (*set_buffer_scale)(int32_t scale, void *user_data);
  void (*damage_buffer)(int32_t x, int32_t y, int32_t width, int32_t height,
                        void *user_data);
  void (*offset)(int32_t x, int32_t y, void *user_data);
} WlSurfaceServerRequestCallbacks;

typedef struct _WlSurfaceServer WlSurfaceServer;

WlSurfaceServer *
wl_surface_server_new(WaylandServerClient *client, uint32_t id,
                      const WlSurfaceServerRequestCallbacks *request_callbacks,
                      void *user_data);

WlSurfaceServer *wl_surface_server_ref(WlSurfaceServer *self);

void wl_surface_server_unref(WlSurfaceServer *self);

void wl_surface_server_enter(WlSurfaceServer *self, uint32_t output);

void wl_surface_server_leave(WlSurfaceServer *self, uint32_t output);

void wl_surface_server_preferred_buffer_scale(WlSurfaceServer *self,
                                              int32_t factor);

void wl_surface_server_preferred_buffer_transform(WlSurfaceServer *self,
                                                  uint32_t transform);
