#include <stdlib.h>

#include "wl_shm_server.h"

struct _WlShmServer {
  const WlShmServerRequestCallbacks *request_callbacks;
  void *user_data;
};

static void wl_shm_create_pool(WlShmServer *self, const uint8_t *payload,
                               uint16_t payload_length) {
  self->request_callbacks->create_pool(self->user_data);
}

static void wl_shm_release(WlShmServer *self, const uint8_t *payload,
                           uint16_t payload_length) {
  self->request_callbacks->release(self->user_data);
}

static void wl_shm_request_cb(uint16_t code, const uint8_t *payload,
                              uint16_t payload_length, void *user_data) {
  WlShmServer *self = user_data;

  switch (code) {
  case 0:
    wl_shm_create_pool(self, payload, payload_length);
    break;
  case 1:
    wl_shm_release(self, payload, payload_length);
    break;
  }
}

WlShmServer *
wl_shm_server_new(WaylandServerClient *client, uint32_t id,
                  const WlShmServerRequestCallbacks *request_callbacks,
                  void *user_data) {
  WlShmServer *self = malloc(sizeof(WlShmServer));
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
