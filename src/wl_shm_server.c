#include <stdlib.h>

#include "wl_shm_server.h"

struct _WlShmServer {
  WaylandServerClient *client;
  uint32_t id;
  const WlShmServerRequestCallbacks *request_callbacks;
  void *user_data;
};

static void wl_shm_create_pool(WlShmServer *self,
                               WaylandPayloadDecoder *decoder) {
  uint32_t id = wayland_payload_decoder_read_new_id(decoder);
  int fd = wayland_payload_decoder_read_fd(decoder);
  int32_t size = wayland_payload_decoder_read_int(decoder);
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->create_pool(id, fd, size, self->user_data);
}

static void wl_shm_release(WlShmServer *self, WaylandPayloadDecoder *decoder) {
  if (!wayland_payload_decoder_finish(decoder)) {
    // FIXME
    return;
  }
  self->request_callbacks->release(self->user_data);
}

static void wl_shm_request_cb(uint16_t code, WaylandPayloadDecoder *decoder,
                              void *user_data) {
  WlShmServer *self = user_data;

  switch (code) {
  case 0:
    wl_shm_create_pool(self, decoder);
    break;
  case 1:
    wl_shm_release(self, decoder);
    break;
  }
}

WlShmServer *
wl_shm_server_new(WaylandServerClient *client, uint32_t id,
                  const WlShmServerRequestCallbacks *request_callbacks,
                  void *user_data) {
  WlShmServer *self = malloc(sizeof(WlShmServer));
  self->client = client;
  self->id = id;
  self->request_callbacks = request_callbacks;
  self->user_data = user_data;

  wayland_server_client_add_object(client, id, wl_shm_request_cb, self);

  return self;
}

WlShmServer *wl_shm_server_ref(WlShmServer *self) {
  // FIXME
  return self;
}

void wl_shm_server_unref(WlShmServer *self) {
  // FIXME
}

void wl_shm_server_format(WlShmServer *self, uint32_t format) {
  WaylandPayloadEncoder *encoder = wayland_payload_encoder_new();
  wayland_payload_encoder_write_uint(encoder, format);
  if (!wayland_payload_encoder_finish(encoder)) {
    // FIXME
  }

  wayland_server_client_send_event(self->client, self->id, 0, encoder);

  wayland_payload_encoder_unref(encoder);
}
