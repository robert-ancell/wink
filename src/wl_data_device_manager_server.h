#pragma once

#include <stdint.h>

#include "wayland_payload_decoder.h"
#include "wayland_server_client.h"

typedef struct {
  void (*create_data_source)(uint32_t id, void *user_data);
  void (*get_data_device)(uint32_t id, uint32_t seat, void *user_data);
} WlDataDeviceManagerServerRequestCallbacks;

typedef struct _WlDataDeviceManagerServer WlDataDeviceManagerServer;

WlDataDeviceManagerServer *wl_data_device_manager_server_new(
    WaylandServerClient *client, uint32_t id,
    const WlDataDeviceManagerServerRequestCallbacks *request_callbacks,
    void *user_data);

WlDataDeviceManagerServer *
wl_data_device_manager_server_ref(WlDataDeviceManagerServer *self);

void wl_data_device_manager_server_unref(WlDataDeviceManagerServer *self);
