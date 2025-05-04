#include <stdlib.h>

#include "wl_data_device_manager_server.h"

struct _WlDataDeviceManagerServer {
  const WlDataDeviceManagerServerRequestCallbacks *request_callbacks;
  void *user_data;
};

static void
wl_data_device_manager_create_data_source(WlDataDeviceManagerServer *self,
                                          WaylandPayloadDecoder *decoder) {
  uint32_t id = wayland_payload_decoder_read_new_id(decoder);
  self->request_callbacks->create_data_source(id, self->user_data);
}

static void
wl_data_device_manager_get_data_device(WlDataDeviceManagerServer *self,
                                       WaylandPayloadDecoder *decoder) {
  uint32_t id = wayland_payload_decoder_read_new_id(decoder);
  uint32_t seat = wayland_payload_decoder_read_object(decoder);
  self->request_callbacks->get_data_device(id, seat, self->user_data);
}

static void wl_data_device_manager_request_cb(uint16_t code,
                                              WaylandPayloadDecoder *decoder,
                                              void *user_data) {
  WlDataDeviceManagerServer *self = user_data;

  switch (code) {
  case 0:
    wl_data_device_manager_create_data_source(self, decoder);
    break;
  case 1:
    wl_data_device_manager_get_data_device(self, decoder);
    break;
  }
}

WlDataDeviceManagerServer *wl_data_device_manager_server_new(
    WaylandServerClient *client, uint32_t id,
    const WlDataDeviceManagerServerRequestCallbacks *request_callbacks,
    void *user_data) {
  WlDataDeviceManagerServer *self = malloc(sizeof(WlDataDeviceManagerServer));
  self->request_callbacks = request_callbacks;
  self->user_data = user_data;

  wayland_server_client_add_object(client, id,
                                   wl_data_device_manager_request_cb, self);

  return self;
}

WlDataDeviceManagerServer *
wl_data_device_manager_server_ref(WlDataDeviceManagerServer *self) {
  // FIXME
  return self;
}

void wl_data_device_manager_server_unref(WlDataDeviceManagerServer *self) {
  // FIXME
}
