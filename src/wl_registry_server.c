#include <stdlib.h>

#include "wl_registry_server.h"

struct _WlRegistryServer {
  WaylandServerClient *client;
  uint32_t id;
  const WlRegistryServerRequestCallbacks *request_callbacks;
  void *user_data;
};

static void wl_registry_bind(WlRegistryServer *self,
                             WaylandPayloadDecoder *decoder) {
  uint32_t name = wayland_payload_decoder_read_uint(decoder);
  const char *id_interface = wayland_payload_decoder_read_string(decoder);
  uint32_t id_version = wayland_payload_decoder_read_uint(decoder);
  uint32_t id = wayland_payload_decoder_read_new_id(decoder);
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->bind(name, id_interface, id_version, id,
                                self->user_data);
}

static void wl_registry_request_cb(uint16_t code,
                                   WaylandPayloadDecoder *decoder,
                                   void *user_data) {
  WlRegistryServer *self = user_data;

  switch (code) {
  case 0:
    wl_registry_bind(self, decoder);
    break;
  }
}

WlRegistryServer *wl_registry_server_new(
    WaylandServerClient *client, uint32_t id,
    const WlRegistryServerRequestCallbacks *request_callbacks,
    void *user_data) {
  WlRegistryServer *self = malloc(sizeof(WlRegistryServer));
  self->client = client;
  self->id = id;
  self->request_callbacks = request_callbacks;
  self->user_data = user_data;

  wayland_server_client_add_object(client, id, wl_registry_request_cb, self);

  return self;
}

WlRegistryServer *wl_registry_server_ref(WlRegistryServer *self) {
  // FIXME
  return self;
}

void wl_registry_server_unref(WlRegistryServer *self) {
  // FIXME
}

void wl_registry_server_global(WlRegistryServer *self, uint32_t name,
                               const char *interface, uint32_t version) {
  WaylandPayloadEncoder *encoder = wayland_payload_encoder_new();
  wayland_payload_encoder_write_uint(encoder, name);
  wayland_payload_encoder_write_string(encoder, interface);
  wayland_payload_encoder_write_uint(encoder, version);
  if (!wayland_payload_encoder_finish(encoder)) {
    // FIXME
  }

  wayland_server_client_send_event(self->client, self->id, 0, encoder);

  wayland_payload_encoder_unref(encoder);
}

void wl_registry_server_global_remove(WlRegistryServer *self, uint32_t name) {
  WaylandPayloadEncoder *encoder = wayland_payload_encoder_new();
  wayland_payload_encoder_write_uint(encoder, name);
  if (!wayland_payload_encoder_finish(encoder)) {
    // FIXME
  }

  wayland_server_client_send_event(self->client, self->id, 1, encoder);

  wayland_payload_encoder_unref(encoder);
}
