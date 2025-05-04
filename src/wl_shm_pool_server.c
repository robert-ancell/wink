#include <stdlib.h>

#include "wl_shm_pool_server.h"

struct _WlShmPoolServer {
  WaylandServerClient *client;
  uint32_t id;
  const WlShmPoolServerRequestCallbacks *request_callbacks;
  void *user_data;
};

static void wl_shm_pool_create_buffer(WlShmPoolServer *self,
                                      WaylandPayloadDecoder *decoder) {
  uint32_t id = wayland_payload_decoder_read_new_id(decoder);
  int32_t offset = wayland_payload_decoder_read_int(decoder);
  int32_t width = wayland_payload_decoder_read_int(decoder);
  int32_t height = wayland_payload_decoder_read_int(decoder);
  int32_t stride = wayland_payload_decoder_read_int(decoder);
  uint32_t format = wayland_payload_decoder_read_uint(decoder);
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->create_buffer(id, offset, width, height, stride,
                                         format, self->user_data);
}

static void wl_shm_pool_destroy(WlShmPoolServer *self,
                                WaylandPayloadDecoder *decoder) {
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->destroy(self->user_data);
}

static void wl_shm_pool_resize(WlShmPoolServer *self,
                               WaylandPayloadDecoder *decoder) {
  int32_t size = wayland_payload_decoder_read_int(decoder);
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->resize(size, self->user_data);
}

static void wl_shm_pool_request_cb(uint16_t code,
                                   WaylandPayloadDecoder *decoder,
                                   void *user_data) {
  WlShmPoolServer *self = user_data;

  switch (code) {
  case 0:
    wl_shm_pool_create_buffer(self, decoder);
    break;
  case 1:
    wl_shm_pool_destroy(self, decoder);
    break;
  case 2:
    wl_shm_pool_resize(self, decoder);
    break;
  }
}

WlShmPoolServer *
wl_shm_pool_server_new(WaylandServerClient *client, uint32_t id,
                       const WlShmPoolServerRequestCallbacks *request_callbacks,
                       void *user_data) {
  WlShmPoolServer *self = malloc(sizeof(WlShmPoolServer));
  self->client = client;
  self->id = id;
  self->request_callbacks = request_callbacks;
  self->user_data = user_data;

  wayland_server_client_add_object(client, id, wl_shm_pool_request_cb, self);

  return self;
}

WlShmPoolServer *wl_shm_pool_server_ref(WlShmPoolServer *self) {
  // FIXME
  return self;
}

void wl_shm_pool_server_unref(WlShmPoolServer *self) {
  // FIXME
}
