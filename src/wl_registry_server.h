#pragma once

#include <stdint.h>

#include "wayland_payload_decoder.h"
#include "wayland_server_client.h"

typedef struct {
  void (*bind)(uint32_t name, const char *id_interface, uint32_t id_version,
               uint32_t id, void *user_data);
} WlRegistryServerRequestCallbacks;

typedef struct _WlRegistryServer WlRegistryServer;

WlRegistryServer *wl_registry_server_new(
    WaylandServerClient *client, uint32_t id,
    const WlRegistryServerRequestCallbacks *request_callbacks, void *user_data);

WlRegistryServer *wl_registry_server_ref(WlRegistryServer *self);

void wl_registry_server_unref(WlRegistryServer *self);

void wl_registry_server_global(WlRegistryServer *self, uint32_t name,
                               const char *interface, uint32_t version);

void wl_registry_server_global_remove(WlRegistryServer *self, uint32_t name);
