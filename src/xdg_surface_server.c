#include <stdlib.h>

#include "xdg_surface_server.h"

struct _XdgSurfaceServer {
  WaylandServerClient *client;
  uint32_t id;
  const XdgSurfaceServerRequestCallbacks *request_callbacks;
  void *user_data;
};

static void xdg_surface_destroy(XdgSurfaceServer *self,
                                WaylandPayloadDecoder *decoder) {
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->destroy(self->user_data);
}

static void xdg_surface_get_toplevel(XdgSurfaceServer *self,
                                     WaylandPayloadDecoder *decoder) {
  uint32_t id = wayland_payload_decoder_read_new_id(decoder);
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->get_toplevel(id, self->user_data);
}

static void xdg_surface_get_popup(XdgSurfaceServer *self,
                                  WaylandPayloadDecoder *decoder) {
  uint32_t id = wayland_payload_decoder_read_new_id(decoder);
  uint32_t parent = wayland_payload_decoder_read_object(decoder);
  uint32_t positioner = wayland_payload_decoder_read_object(decoder);
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->get_popup(id, parent, positioner, self->user_data);
}

static void xdg_surface_set_window_geometry(XdgSurfaceServer *self,
                                            WaylandPayloadDecoder *decoder) {
  int32_t x = wayland_payload_decoder_read_int(decoder);
  int32_t y = wayland_payload_decoder_read_int(decoder);
  int32_t width = wayland_payload_decoder_read_int(decoder);
  int32_t height = wayland_payload_decoder_read_int(decoder);
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->set_window_geometry(x, y, width, height,
                                               self->user_data);
}

static void xdg_surface_ack_configure(XdgSurfaceServer *self,
                                      WaylandPayloadDecoder *decoder) {
  uint32_t serial = wayland_payload_decoder_read_uint(decoder);
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->ack_configure(serial, self->user_data);
}

static void xdg_surface_request_cb(uint16_t code,
                                   WaylandPayloadDecoder *decoder,
                                   void *user_data) {
  XdgSurfaceServer *self = user_data;

  switch (code) {
  case 0:
    xdg_surface_destroy(self, decoder);
    break;
  case 1:
    xdg_surface_get_toplevel(self, decoder);
    break;
  case 2:
    xdg_surface_get_popup(self, decoder);
    break;
  case 3:
    xdg_surface_set_window_geometry(self, decoder);
    break;
  case 4:
    xdg_surface_ack_configure(self, decoder);
    break;
  }
}

XdgSurfaceServer *xdg_surface_server_new(
    WaylandServerClient *client, uint32_t id,
    const XdgSurfaceServerRequestCallbacks *request_callbacks,
    void *user_data) {
  XdgSurfaceServer *self = malloc(sizeof(XdgSurfaceServer));
  self->client = client;
  self->id = id;
  self->request_callbacks = request_callbacks;
  self->user_data = user_data;

  wayland_server_client_add_object(client, id, xdg_surface_request_cb, self);

  return self;
}

XdgSurfaceServer *xdg_surface_server_ref(XdgSurfaceServer *self) {
  // FIXME
  return self;
}

void xdg_surface_server_unref(XdgSurfaceServer *self) {
  // FIXME
}

void xdg_surface_server_configure(XdgSurfaceServer *self, uint32_t serial) {
  WaylandPayloadEncoder *encoder = wayland_payload_encoder_new();
  wayland_payload_encoder_write_uint(encoder, serial);
  if (!wayland_payload_encoder_finish(encoder)) {
    // FIXME
  }

  wayland_server_client_send_event(self->client, self->id, 0, encoder);

  wayland_payload_encoder_unref(encoder);
}
