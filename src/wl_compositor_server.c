#include <stdlib.h>

#include "wl_compositor_server.h"

struct _WlCompositorServer {
  const WlCompositorServerRequestCallbacks *request_callbacks;
  void *user_data;
};

static void wl_compositor_create_surface(WlCompositorServer *self,
                                         const uint8_t *payload,
                                         uint16_t payload_length) {
  uint32_t id;
  self->request_callbacks->create_surface(id, self->user_data);
}

static void wl_compositor_create_region(WlCompositorServer *self,
                                        const uint8_t *payload,
                                        uint16_t payload_length) {
  uint32_t id;
  self->request_callbacks->create_region(id, self->user_data);
}

static void wl_compositor_request_cb(uint16_t code, const uint8_t *payload,
                                     uint16_t payload_length, void *user_data) {
  WlCompositorServer *self = user_data;

  switch (code) {
  case 0:
    wl_compositor_create_surface(self, payload, payload_length);
    break;
  case 1:
    wl_compositor_create_region(self, payload, payload_length);
    break;
  }
}

WlCompositorServer *wl_compositor_server_new(
    WaylandServerClient *client, uint32_t id,
    const WlCompositorServerRequestCallbacks *request_callbacks,
    void *user_data) {
  WlCompositorServer *self = malloc(sizeof(WlCompositorServer));
  self->request_callbacks = request_callbacks;
  self->user_data = user_data;

  wayland_server_client_add_object(client, id, wl_compositor_request_cb, self);

  return self;
}

WlCompositorServer *wl_compositor_server_ref(WlCompositorServer *self) {
  // FIXME
  return self;
}

void wl_compositor_server_unref(WlCompositorServer *self) {
  // FIXME
}
