#include <stdlib.h>

#include "wl_surface_server.h"

struct _WlSurfaceServer {
  WaylandServerClient *client;
  uint32_t id;
  const WlSurfaceServerRequestCallbacks *request_callbacks;
  void *user_data;
};

static void wl_surface_destroy(WlSurfaceServer *self,
                               WaylandPayloadDecoder *decoder) {
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->destroy(self->user_data);
}

static void wl_surface_attach(WlSurfaceServer *self,
                              WaylandPayloadDecoder *decoder) {
  uint32_t buffer = wayland_payload_decoder_read_object(decoder);
  int32_t x = wayland_payload_decoder_read_int(decoder);
  int32_t y = wayland_payload_decoder_read_int(decoder);
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->attach(buffer, x, y, self->user_data);
}

static void wl_surface_damage(WlSurfaceServer *self,
                              WaylandPayloadDecoder *decoder) {
  int32_t x = wayland_payload_decoder_read_int(decoder);
  int32_t y = wayland_payload_decoder_read_int(decoder);
  int32_t width = wayland_payload_decoder_read_int(decoder);
  int32_t height = wayland_payload_decoder_read_int(decoder);
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->damage(x, y, width, height, self->user_data);
}

static void wl_surface_frame(WlSurfaceServer *self,
                             WaylandPayloadDecoder *decoder) {
  uint32_t callback = wayland_payload_decoder_read_new_id(decoder);
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->frame(callback, self->user_data);
}

static void wl_surface_set_opaque_region(WlSurfaceServer *self,
                                         WaylandPayloadDecoder *decoder) {
  uint32_t region = wayland_payload_decoder_read_object(decoder);
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->set_opaque_region(region, self->user_data);
}

static void wl_surface_set_input_region(WlSurfaceServer *self,
                                        WaylandPayloadDecoder *decoder) {
  uint32_t region = wayland_payload_decoder_read_object(decoder);
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->set_input_region(region, self->user_data);
}

static void wl_surface_commit(WlSurfaceServer *self,
                              WaylandPayloadDecoder *decoder) {
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->commit(self->user_data);
}

static void wl_surface_set_buffer_transform(WlSurfaceServer *self,
                                            WaylandPayloadDecoder *decoder) {
  int32_t transform = wayland_payload_decoder_read_int(decoder);
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->set_buffer_transform(transform, self->user_data);
}

static void wl_surface_set_buffer_scale(WlSurfaceServer *self,
                                        WaylandPayloadDecoder *decoder) {
  int32_t scale = wayland_payload_decoder_read_int(decoder);
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->set_buffer_scale(scale, self->user_data);
}

static void wl_surface_damage_buffer(WlSurfaceServer *self,
                                     WaylandPayloadDecoder *decoder) {
  int32_t x = wayland_payload_decoder_read_int(decoder);
  int32_t y = wayland_payload_decoder_read_int(decoder);
  int32_t width = wayland_payload_decoder_read_int(decoder);
  int32_t height = wayland_payload_decoder_read_int(decoder);
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->damage_buffer(x, y, width, height, self->user_data);
}

static void wl_surface_offset(WlSurfaceServer *self,
                              WaylandPayloadDecoder *decoder) {
  int32_t x = wayland_payload_decoder_read_int(decoder);
  int32_t y = wayland_payload_decoder_read_int(decoder);
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->offset(x, y, self->user_data);
}

static void wl_surface_request_cb(uint16_t code, WaylandPayloadDecoder *decoder,
                                  void *user_data) {
  WlSurfaceServer *self = user_data;

  switch (code) {
  case 0:
    wl_surface_destroy(self, decoder);
    break;
  case 1:
    wl_surface_attach(self, decoder);
    break;
  case 2:
    wl_surface_damage(self, decoder);
    break;
  case 3:
    wl_surface_frame(self, decoder);
    break;
  case 4:
    wl_surface_set_opaque_region(self, decoder);
    break;
  case 5:
    wl_surface_set_input_region(self, decoder);
    break;
  case 6:
    wl_surface_commit(self, decoder);
    break;
  case 7:
    wl_surface_set_buffer_transform(self, decoder);
    break;
  case 8:
    wl_surface_set_buffer_scale(self, decoder);
    break;
  case 9:
    wl_surface_damage_buffer(self, decoder);
    break;
  case 10:
    wl_surface_offset(self, decoder);
    break;
  }
}

WlSurfaceServer *
wl_surface_server_new(WaylandServerClient *client, uint32_t id,
                      const WlSurfaceServerRequestCallbacks *request_callbacks,
                      void *user_data) {
  WlSurfaceServer *self = malloc(sizeof(WlSurfaceServer));
  self->client = client;
  self->id = id;
  self->request_callbacks = request_callbacks;
  self->user_data = user_data;

  wayland_server_client_add_object(client, id, wl_surface_request_cb, self);

  return self;
}

WlSurfaceServer *wl_surface_server_ref(WlSurfaceServer *self) {
  // FIXME
  return self;
}

void wl_surface_server_unref(WlSurfaceServer *self) {
  // FIXME
}

void wl_surface_server_enter(WlSurfaceServer *self, uint32_t output) {
  WaylandPayloadEncoder *encoder = wayland_payload_encoder_new();
  wayland_payload_encoder_write_object(encoder, output);
  if (!wayland_payload_encoder_finish(encoder)) {
    // FIXME
  }

  wayland_server_client_send_event(self->client, self->id, 0, encoder);

  wayland_payload_encoder_unref(encoder);
}

void wl_surface_server_leave(WlSurfaceServer *self, uint32_t output) {
  WaylandPayloadEncoder *encoder = wayland_payload_encoder_new();
  wayland_payload_encoder_write_object(encoder, output);
  if (!wayland_payload_encoder_finish(encoder)) {
    // FIXME
  }

  wayland_server_client_send_event(self->client, self->id, 1, encoder);

  wayland_payload_encoder_unref(encoder);
}

void wl_surface_server_preferred_buffer_scale(WlSurfaceServer *self,
                                              int32_t factor) {
  WaylandPayloadEncoder *encoder = wayland_payload_encoder_new();
  wayland_payload_encoder_write_int(encoder, factor);
  if (!wayland_payload_encoder_finish(encoder)) {
    // FIXME
  }

  wayland_server_client_send_event(self->client, self->id, 2, encoder);

  wayland_payload_encoder_unref(encoder);
}

void wl_surface_server_preferred_buffer_transform(WlSurfaceServer *self,
                                                  uint32_t transform) {
  WaylandPayloadEncoder *encoder = wayland_payload_encoder_new();
  wayland_payload_encoder_write_uint(encoder, transform);
  if (!wayland_payload_encoder_finish(encoder)) {
    // FIXME
  }

  wayland_server_client_send_event(self->client, self->id, 3, encoder);

  wayland_payload_encoder_unref(encoder);
}
