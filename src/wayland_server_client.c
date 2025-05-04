#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "wayland_server_client.h"
#include "wl_callback_server.h"
#include "wl_compositor_server.h"
#include "wl_data_device_manager_server.h"
#include "wl_display_server.h"
#include "wl_registry_server.h"
#include "wl_shm_server.h"
#include "xdg_wm_base_server.h"

#define WL_DISPLAY_ID 1

typedef struct {
  uint32_t id;
  WaylandServerClientRequestCallback request_callback;
  void *user_data;
} WaylandObject;

struct _WaylandServerClient {
  int fd;
  WaylandObject *objects;
  size_t objects_length;
};

static void wl_compositor_create_surface(uint32_t id, void *user_data) {
  printf("wl_compositor::create_surface %d\n", id);
}

static void wl_compositor_create_region(uint32_t id, void *user_data) {
  printf("wl_compositor::create_region %d\n", id);
}

static WlCompositorServerRequestCallbacks wl_compositor_request_callbacks = {
    .create_surface = wl_compositor_create_surface,
    .create_region = wl_compositor_create_region};

static void wl_shm_create_pool(uint32_t id, int fd, int32_t size,
                               void *user_data) {
  // FIXME
  printf("wl_shm::create_pool %d\n", id);
}

static void wl_shm_release(void *user_data) { printf("wl_shm::release\n"); }

static WlShmServerRequestCallbacks wl_shm_request_callbacks = {
    .create_pool = wl_shm_create_pool, .release = wl_shm_release};

static void wl_data_device_manager_create_data_source(uint32_t id,
                                                      void *user_data) {
  printf("wl_data_device_manager::create_data_source\n");
}

static void wl_data_device_manager_get_data_device(uint32_t id, uint32_t seat,
                                                   void *user_data) {
  printf("wl_data_device_manager::get_data_device\n");
}

static WlDataDeviceManagerServerRequestCallbacks
    wl_data_device_manager_request_callbacks = {
        .create_data_source = wl_data_device_manager_create_data_source,
        .get_data_device = wl_data_device_manager_get_data_device};

static void xdg_wm_base_destroy(void *user_data) {
  printf("xdg_wm_base::destroy\n");
}

static void xdg_wm_base_create_positioner(uint32_t id, void *user_data) {
  printf("xdg_wm_base::create_positioner\n");
}

static void xdg_wm_base_get_xdg_surface(uint32_t id, uint32_t surface,
                                        void *user_data) {
  printf("xdg_wm_base::get_xdg_surface %d %d\n", id, surface);
}

static void xdg_wm_base_pong(uint32_t serial, void *user_data) {
  printf("xdg_wm_base::pong %d\n", serial);
}

static XdgWmBaseServerRequestCallbacks xdg_wm_base_request_callbacks = {
    .destroy = xdg_wm_base_destroy,
    .create_positioner = xdg_wm_base_create_positioner,
    .get_xdg_surface = xdg_wm_base_get_xdg_surface,
    .pong = xdg_wm_base_pong};

static WlCallbackServerRequestCallbacks wl_callback_request_callbacks = {};

static void wl_registry_bind(uint32_t name, const char *id_interface,
                             uint32_t id_version, uint32_t id,
                             void *user_data) {
  WaylandServerClient *self = user_data;

  printf("wl_registry::bind %d %s %d %d\n", name, id_interface, id_version, id);

  switch (name) {
  case 1:
    // FIXME: Store object
    wl_compositor_server_new(self, id, &wl_compositor_request_callbacks, self);
    break;
  case 2:
    // FIXME: Store object
    wl_shm_server_new(self, id, &wl_shm_request_callbacks, self);
    break;
  case 3:
    // FIXME: Store object
    wl_data_device_manager_server_new(
        self, id, &wl_data_device_manager_request_callbacks, self);
    break;
  case 4:
    // FIXME: Store object
    xdg_wm_base_server_new(self, id, &xdg_wm_base_request_callbacks, self);
    break;
  }
}

static WlRegistryServerRequestCallbacks wl_registry_request_callbacks = {
    .bind = wl_registry_bind};

static void wl_display_sync(uint32_t callback, void *user_data) {
  WaylandServerClient *self = user_data;

  printf("wl_display::sync %d\n", callback);

  WlCallbackServer *object = wl_callback_server_new(
      self, callback, &wl_callback_request_callbacks, self);

  wl_callback_server_done(object, 0);
}

static void wl_display_get_registry(uint32_t id, void *user_data) {
  WaylandServerClient *self = user_data;

  printf("wl_display::get_registry %d\n", id);

  WlRegistryServer *registry =
      wl_registry_server_new(self, id, &wl_registry_request_callbacks, self);
  wl_registry_server_global(registry, 1, "wl_compositor", 6);
  wl_registry_server_global(registry, 2, "wl_shm", 2);
  wl_registry_server_global(registry, 3, "wl_data_device_manager", 3);
  wl_registry_server_global(registry, 4, "xdg_wm_base", 6);
}

static WlDisplayServerRequestCallbacks wl_display_request_callbacks = {
    .sync = wl_display_sync, .get_registry = wl_display_get_registry};

static WaylandObject *find_object(WaylandServerClient *self, uint32_t id) {
  // FIXME: Binary search
  for (size_t i = 0; i < self->objects_length; i++) {
    WaylandObject *o = &self->objects[i];
    if (o->id == id) {
      return o;
    }
  }

  return NULL;
}

static void decode_request(WaylandServerClient *self, uint32_t id,
                           uint16_t code, WaylandPayloadDecoder *decoder) {
  WaylandObject *o = find_object(self, id);
  if (o == NULL) {
    // FIXME: Generate error
    printf("unknown object %d\n", id);
    return;
  }

  o->request_callback(code, decoder, o->user_data);
}

static void read_cb(void *user_data) {
  WaylandServerClient *self = user_data;

  uint8_t data[1024];
  ssize_t data_length = read(self->fd, data, 1024);
  if (data_length == -1) {
    return;
  }

  size_t offset = 0;
  while (offset + 8 <= data_length) {
    uint32_t *header = (uint32_t *)(data + offset);
    uint32_t length_code = header[1];
    uint16_t length = length_code >> 16;

    if (length < 8) {
      // FIXME: Invalid
      break;
    }
    if (offset + length > data_length) {
      break;
    }

    uint32_t id = header[0];
    uint16_t code = header[1] & 0xffff;
    uint8_t *payload = data + offset + 8;
    WaylandPayloadDecoder *decoder =
        wayland_payload_decoder_new(payload, length - 8);
    decode_request(self, id, code, decoder);
    wayland_payload_decoder_unref(decoder);

    offset += length;
  }
}

WaylandServerClient *wayland_server_client_new(MainLoop *loop, int fd) {
  WaylandServerClient *self = malloc(sizeof(WaylandServerClient));
  self->fd = fd;
  self->objects = NULL;
  self->objects_length = 0;

  main_loop_add_fd(loop, fd, read_cb, self);

  wl_display_server_new(self, WL_DISPLAY_ID, &wl_display_request_callbacks,
                        self);

  return self;
}

WaylandServerClient *wayland_server_client_ref(WaylandServerClient *self) {
  // FIXME
  return self;
}

void wayland_server_client_unref(WaylandServerClient *self) {
  // FIXME
}

void wayland_server_client_add_object(
    WaylandServerClient *self, uint32_t id,
    WaylandServerClientRequestCallback request_callback, void *user_data) {
  if (find_object(self, id) != NULL) {
    // FIXME: error
    return;
  }

  self->objects_length++;
  self->objects =
      realloc(self->objects, sizeof(WaylandObject) * self->objects_length);
  WaylandObject *o = &self->objects[self->objects_length - 1];
  o->id = id;
  o->request_callback = request_callback;
  o->user_data = user_data;
}

void wayland_server_client_send_event(WaylandServerClient *self, uint32_t id,
                                      uint16_t code,
                                      WaylandPayloadEncoder *encoder) {
  size_t payload_length = wayland_payload_encoder_get_length(encoder);
  // FIXME: Check may payload length (65535-8)
  uint32_t header[2];
  header[0] = id;
  header[1] = (payload_length + 8) << 16 | code;
  // FIXME: Handle partial writes
  write(self->fd, header, 8);
  write(self->fd, wayland_payload_encoder_get_data(encoder), payload_length);
}
