#include <stdlib.h>

#include "xdg_wm_base_server.h"

struct _XdgWmBaseServer {
  const XdgWmBaseServerRequestCallbacks *request_callbacks;
  void *user_data;
};

static void xdg_wm_base_destroy(XdgWmBaseServer *self,
                                WaylandPayloadDecoder *decoder) {
  self->request_callbacks->destroy(self->user_data);
}

static void xdg_wm_base_create_positioner(XdgWmBaseServer *self,
                                          WaylandPayloadDecoder *decoder) {
  uint32_t id = wayland_payload_decoder_read_new_id(decoder);
  self->request_callbacks->create_positioner(id, self->user_data);
}

static void xdg_wm_base_get_xdg_surface(XdgWmBaseServer *self,
                                        WaylandPayloadDecoder *decoder) {
  uint32_t id = wayland_payload_decoder_read_new_id(decoder);
  uint32_t surface = wayland_payload_decoder_read_object(decoder);
  self->request_callbacks->get_xdg_surface(id, surface, self->user_data);
}

static void xdg_wm_base_pong(XdgWmBaseServer *self,
                             WaylandPayloadDecoder *decoder) {
  uint32_t serial = wayland_payload_decoder_read_uint(decoder);
  self->request_callbacks->pong(serial, self->user_data);
}

static void xdg_wm_base_request_cb(uint16_t code,
                                   WaylandPayloadDecoder *decoder,
                                   void *user_data) {
  XdgWmBaseServer *self = user_data;

  switch (code) {
  case 0:
    xdg_wm_base_destroy(self, decoder);
    break;
  case 1:
    xdg_wm_base_create_positioner(self, decoder);
    break;
  case 2:
    xdg_wm_base_get_xdg_surface(self, decoder);
    break;
  case 3:
    xdg_wm_base_pong(self, decoder);
    break;
  }
}

XdgWmBaseServer *
xdg_wm_base_server_new(WaylandServerClient *client, uint32_t id,
                       const XdgWmBaseServerRequestCallbacks *request_callbacks,
                       void *user_data) {
  XdgWmBaseServer *self = malloc(sizeof(XdgWmBaseServer));
  self->request_callbacks = request_callbacks;
  self->user_data = user_data;

  wayland_server_client_add_object(client, id, xdg_wm_base_request_cb, self);

  return self;
}

XdgWmBaseServer *xdg_wm_base_server_ref(XdgWmBaseServer *self) {
  // FIXME
  return self;
}

void xdg_wm_base_server_unref(XdgWmBaseServer *self) {
  // FIXME
}

void xdg_wm_base_server_ping(XdgWmBaseServer *self, uint32_t serial) {
  // FIXME
}
