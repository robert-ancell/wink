#include <stdlib.h>

#include "wl_display_server.h"

struct _WlDisplayServer {
  WaylandServerClient *client;
  uint32_t id;
  const WlDisplayServerRequestCallbacks *request_callbacks;
  void *user_data;
};

static void wl_display_sync(WlDisplayServer *self,
                            WaylandPayloadDecoder *decoder) {
  uint32_t callback = wayland_payload_decoder_read_new_id(decoder);
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->sync(callback, self->user_data);
}

static void wl_display_get_registry(WlDisplayServer *self,
                                    WaylandPayloadDecoder *decoder) {
  uint32_t registry = wayland_payload_decoder_read_new_id(decoder);
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->get_registry(registry, self->user_data);
}

static void wl_display_request_cb(uint16_t code, WaylandPayloadDecoder *decoder,
                                  void *user_data) {
  WlDisplayServer *self = user_data;

  switch (code) {
  case 0:
    wl_display_sync(self, decoder);
    break;
  case 1:
    wl_display_get_registry(self, decoder);
    break;
  }
}

WlDisplayServer *
wl_display_server_new(WaylandServerClient *client, uint32_t id,
                      const WlDisplayServerRequestCallbacks *request_callbacks,
                      void *user_data) {
  WlDisplayServer *self = malloc(sizeof(WlDisplayServer));
  self->client = client;
  self->id = id;
  self->request_callbacks = request_callbacks;
  self->user_data = user_data;

  wayland_server_client_add_object(client, id, wl_display_request_cb, self);

  return self;
}

WlDisplayServer *wl_display_server_ref(WlDisplayServer *self) {
  // FIXME
  return self;
}

void wl_display_server_unref(WlDisplayServer *self) {
  // FIXME
}

void wl_display_server_error(WlDisplayServer *self, uint32_t object_id,
                             uint32_t code, const char *message) {
  WaylandPayloadEncoder *encoder = wayland_payload_encoder_new();
  wayland_payload_encoder_write_object(encoder, object_id);
  wayland_payload_encoder_write_uint(encoder, code);
  wayland_payload_encoder_write_string(encoder, message);
  if (!wayland_payload_encoder_finish(encoder)) {
    // FIXME
  }

  wayland_server_client_send_event(self->client, self->id, 0, encoder);

  wayland_payload_encoder_unref(encoder);
}

void wl_display_server_delete_id(WlDisplayServer *self, uint32_t id) {
  WaylandPayloadEncoder *encoder = wayland_payload_encoder_new();
  wayland_payload_encoder_write_uint(encoder, id);
  if (!wayland_payload_encoder_finish(encoder)) {
    // FIXME
  }

  wayland_server_client_send_event(self->client, self->id, 1, encoder);

  wayland_payload_encoder_unref(encoder);
}
