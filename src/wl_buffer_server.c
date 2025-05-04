#include <stdlib.h>

#include "wl_buffer_server.h"

struct _WlBufferServer {
  WaylandServerClient *client;
  uint32_t id;
  const WlBufferServerRequestCallbacks *request_callbacks;
  void *user_data;
};

static void wl_buffer_destroy(WlBufferServer *self,
                              WaylandPayloadDecoder *decoder) {
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->destroy(self->user_data);
}

static void wl_buffer_request_cb(uint16_t code, WaylandPayloadDecoder *decoder,
                                 void *user_data) {
  WlBufferServer *self = user_data;

  switch (code) {
  case 0:
    wl_buffer_destroy(self, decoder);
    break;
  }
}

WlBufferServer *
wl_buffer_server_new(WaylandServerClient *client, uint32_t id,
                     const WlBufferServerRequestCallbacks *request_callbacks,
                     void *user_data) {
  WlBufferServer *self = malloc(sizeof(WlBufferServer));
  self->client = client;
  self->id = id;
  self->request_callbacks = request_callbacks;
  self->user_data = user_data;

  wayland_server_client_add_object(client, id, wl_buffer_request_cb, self);

  return self;
}

WlBufferServer *wl_buffer_server_ref(WlBufferServer *self) {
  // FIXME
  return self;
}

void wl_buffer_server_unref(WlBufferServer *self) {
  // FIXME
}

void wl_buffer_server_release(WlBufferServer *self) {
  WaylandPayloadEncoder *encoder = wayland_payload_encoder_new();
  if (!wayland_payload_encoder_finish(encoder)) {
    // FIXME
  }

  wayland_server_client_send_event(self->client, self->id, 0, encoder);

  wayland_payload_encoder_unref(encoder);
}
