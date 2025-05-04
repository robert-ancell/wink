#include <stdlib.h>

#include "wl_callback_server.h"

struct _WlCallbackServer {
  WaylandServerClient *client;
  uint32_t id;
  const WlCallbackServerRequestCallbacks *request_callbacks;
  void *user_data;
};

static void wl_callback_request_cb(uint16_t code,
                                   WaylandPayloadDecoder *decoder,
                                   void *user_data) {}

WlCallbackServer *wl_callback_server_new(
    WaylandServerClient *client, uint32_t id,
    const WlCallbackServerRequestCallbacks *request_callbacks,
    void *user_data) {
  WlCallbackServer *self = malloc(sizeof(WlCallbackServer));
  self->client = client;
  self->id = id;
  self->request_callbacks = request_callbacks;
  self->user_data = user_data;

  wayland_server_client_add_object(client, id, wl_callback_request_cb, self);

  return self;
}

WlCallbackServer *wl_callback_server_ref(WlCallbackServer *self) {
  // FIXME
  return self;
}

void wl_callback_server_unref(WlCallbackServer *self) {
  // FIXME
}

void wl_callback_server_done(WlCallbackServer *self, uint32_t callback_data) {
  WaylandPayloadEncoder *encoder = wayland_payload_encoder_new();
  wayland_payload_encoder_write_uint(encoder, callback_data);
  if (!wayland_payload_encoder_finish(encoder)) {
    // FIXME
  }

  wayland_server_client_send_event(self->client, self->id, 0, encoder);

  wayland_payload_encoder_unref(encoder);
}
