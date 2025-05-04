#include <stdlib.h>

#include "xdg_toplevel_server.h"

struct _XdgToplevelServer {
  WaylandServerClient *client;
  uint32_t id;
  const XdgToplevelServerRequestCallbacks *request_callbacks;
  void *user_data;
};

static void xdg_toplevel_destroy(XdgToplevelServer *self,
                                 WaylandPayloadDecoder *decoder) {
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->destroy(self->user_data);
}

static void xdg_toplevel_set_parent(XdgToplevelServer *self,
                                    WaylandPayloadDecoder *decoder) {
  uint32_t parent = wayland_payload_decoder_read_object(decoder);
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->set_parent(parent, self->user_data);
}

static void xdg_toplevel_set_title(XdgToplevelServer *self,
                                   WaylandPayloadDecoder *decoder) {
  const char *title = wayland_payload_decoder_read_string(decoder);
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->set_title(title, self->user_data);
}

static void xdg_toplevel_set_app_id(XdgToplevelServer *self,
                                    WaylandPayloadDecoder *decoder) {
  const char *app_id = wayland_payload_decoder_read_string(decoder);
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->set_app_id(app_id, self->user_data);
}

static void xdg_toplevel_show_window_menu(XdgToplevelServer *self,
                                          WaylandPayloadDecoder *decoder) {
  uint32_t seat = wayland_payload_decoder_read_object(decoder);
  uint32_t serial = wayland_payload_decoder_read_uint(decoder);
  int32_t x = wayland_payload_decoder_read_int(decoder);
  int32_t y = wayland_payload_decoder_read_int(decoder);
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->show_window_menu(seat, serial, x, y,
                                            self->user_data);
}

static void xdg_toplevel_move(XdgToplevelServer *self,
                              WaylandPayloadDecoder *decoder) {
  uint32_t seat = wayland_payload_decoder_read_object(decoder);
  uint32_t serial = wayland_payload_decoder_read_uint(decoder);
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->move(seat, serial, self->user_data);
}

static void xdg_toplevel_resize(XdgToplevelServer *self,
                                WaylandPayloadDecoder *decoder) {
  uint32_t seat = wayland_payload_decoder_read_object(decoder);
  uint32_t serial = wayland_payload_decoder_read_uint(decoder);
  uint32_t edges = wayland_payload_decoder_read_uint(decoder);
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->resize(seat, serial, edges, self->user_data);
}

static void xdg_toplevel_set_max_size(XdgToplevelServer *self,
                                      WaylandPayloadDecoder *decoder) {
  int32_t width = wayland_payload_decoder_read_int(decoder);
  int32_t height = wayland_payload_decoder_read_int(decoder);
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->set_max_size(width, height, self->user_data);
}

static void xdg_toplevel_set_min_size(XdgToplevelServer *self,
                                      WaylandPayloadDecoder *decoder) {
  int32_t width = wayland_payload_decoder_read_int(decoder);
  int32_t height = wayland_payload_decoder_read_int(decoder);
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->set_min_size(width, height, self->user_data);
}

static void xdg_toplevel_set_maximized(XdgToplevelServer *self,
                                       WaylandPayloadDecoder *decoder) {
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->set_maximized(self->user_data);
}

static void xdg_toplevel_unset_maximized(XdgToplevelServer *self,
                                         WaylandPayloadDecoder *decoder) {
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->unset_maximized(self->user_data);
}

static void xdg_toplevel_set_fullscreen(XdgToplevelServer *self,
                                        WaylandPayloadDecoder *decoder) {
  uint32_t output = wayland_payload_decoder_read_object(decoder);
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->set_fullscreen(output, self->user_data);
}

static void xdg_toplevel_unset_fullscreen(XdgToplevelServer *self,
                                          WaylandPayloadDecoder *decoder) {
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->unset_fullscreen(self->user_data);
}

static void xdg_toplevel_set_minimized(XdgToplevelServer *self,
                                       WaylandPayloadDecoder *decoder) {
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->set_minimized(self->user_data);
}

static void xdg_toplevel_request_cb(uint16_t code,
                                    WaylandPayloadDecoder *decoder,
                                    void *user_data) {
  XdgToplevelServer *self = user_data;

  switch (code) {
  case 0:
    xdg_toplevel_destroy(self, decoder);
    break;
  case 1:
    xdg_toplevel_set_parent(self, decoder);
    break;
  case 2:
    xdg_toplevel_set_title(self, decoder);
    break;
  case 3:
    xdg_toplevel_set_app_id(self, decoder);
    break;
  case 4:
    xdg_toplevel_show_window_menu(self, decoder);
    break;
  case 5:
    xdg_toplevel_move(self, decoder);
    break;
  case 6:
    xdg_toplevel_resize(self, decoder);
    break;
  case 7:
    xdg_toplevel_set_max_size(self, decoder);
    break;
  case 8:
    xdg_toplevel_set_min_size(self, decoder);
    break;
  case 9:
    xdg_toplevel_set_maximized(self, decoder);
    break;
  case 10:
    xdg_toplevel_unset_maximized(self, decoder);
    break;
  case 11:
    xdg_toplevel_set_fullscreen(self, decoder);
    break;
  case 12:
    xdg_toplevel_unset_fullscreen(self, decoder);
    break;
  case 13:
    xdg_toplevel_set_minimized(self, decoder);
    break;
  }
}

XdgToplevelServer *xdg_toplevel_server_new(
    WaylandServerClient *client, uint32_t id,
    const XdgToplevelServerRequestCallbacks *request_callbacks,
    void *user_data) {
  XdgToplevelServer *self = malloc(sizeof(XdgToplevelServer));
  self->client = client;
  self->id = id;
  self->request_callbacks = request_callbacks;
  self->user_data = user_data;

  wayland_server_client_add_object(client, id, xdg_toplevel_request_cb, self);

  return self;
}

XdgToplevelServer *xdg_toplevel_server_ref(XdgToplevelServer *self) {
  // FIXME
  return self;
}

void xdg_toplevel_server_unref(XdgToplevelServer *self) {
  // FIXME
}

void xdg_toplevel_server_configure(XdgToplevelServer *self, int32_t width,
                                   int32_t height, uint32_t *states) {
  WaylandPayloadEncoder *encoder = wayland_payload_encoder_new();
  wayland_payload_encoder_write_int(encoder, width);
  wayland_payload_encoder_write_int(encoder, height);
  wayland_payload_encoder_write_array(encoder, states);
  if (!wayland_payload_encoder_finish(encoder)) {
    // FIXME
  }

  wayland_server_client_send_event(self->client, self->id, 0, encoder);

  wayland_payload_encoder_unref(encoder);
}

void xdg_toplevel_server_close(XdgToplevelServer *self) {
  WaylandPayloadEncoder *encoder = wayland_payload_encoder_new();
  if (!wayland_payload_encoder_finish(encoder)) {
    // FIXME
  }

  wayland_server_client_send_event(self->client, self->id, 1, encoder);

  wayland_payload_encoder_unref(encoder);
}

void xdg_toplevel_server_configure_bounds(XdgToplevelServer *self,
                                          int32_t width, int32_t height) {
  WaylandPayloadEncoder *encoder = wayland_payload_encoder_new();
  wayland_payload_encoder_write_int(encoder, width);
  wayland_payload_encoder_write_int(encoder, height);
  if (!wayland_payload_encoder_finish(encoder)) {
    // FIXME
  }

  wayland_server_client_send_event(self->client, self->id, 2, encoder);

  wayland_payload_encoder_unref(encoder);
}

void xdg_toplevel_server_wm_capabilities(XdgToplevelServer *self,
                                         uint32_t *capabilities) {
  WaylandPayloadEncoder *encoder = wayland_payload_encoder_new();
  wayland_payload_encoder_write_array(encoder, capabilities);
  if (!wayland_payload_encoder_finish(encoder)) {
    // FIXME
  }

  wayland_server_client_send_event(self->client, self->id, 3, encoder);

  wayland_payload_encoder_unref(encoder);
}
