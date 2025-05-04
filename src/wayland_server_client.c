#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "wayland_server_client.h"
#include "wl_compositor_server.h"
#include "wl_data_device_manager_server.h"
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

static size_t set_uint(uint8_t *payload, size_t offset, uint32_t value) {
  *(uint32_t *)(payload + offset) = value;
  return offset + 4;
}

static size_t string_length(const char *value) {
  size_t n_words = (strlen(value) + 1 + 3) / 4;
  return 4 + n_words * 4;
}

static size_t set_string(uint8_t *payload, size_t offset, const char *value) {
  offset = set_uint(payload, offset, strlen(value) + 1);
  for (size_t i = 0; value[i] != '\0'; i++) {
    payload[offset++] = value[i];
  }
  payload[offset++] = '\0';
  while (offset % 4 != 0) {
    payload[offset++] = '\0';
  }

  return offset;
}

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

static void wl_callback_request_cb(uint16_t code,
                                   WaylandPayloadDecoder *decoder,
                                   void *user_data) {}

static void wl_callback_done(WaylandServerClient *self, uint32_t id,
                             uint32_t callback_data) {
  uint8_t payload[4];
  set_uint(payload, 0, callback_data);
  wayland_server_client_send_event(self, id, 0, payload, sizeof(payload));
}

static void wl_registry_bind(WaylandServerClient *self,
                             WaylandPayloadDecoder *decoder) {
  uint32_t name = wayland_payload_decoder_read_uint(decoder);
  const char *interface = wayland_payload_decoder_read_string(decoder);
  uint32_t version = wayland_payload_decoder_read_uint(decoder);
  uint32_t id = wayland_payload_decoder_read_uint(decoder);
  if (!wayland_payload_decoder_finish(decoder)) {
    return;
  }

  printf("wl_registry::bind %d %s %d %d\n", name, interface, version, id);

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

static void wl_registry_request_cb(uint16_t code,
                                   WaylandPayloadDecoder *decoder,
                                   void *user_data) {
  WaylandServerClient *self = user_data;

  switch (code) {
  case 0:
    wl_registry_bind(self, decoder);
    break;
  }
}

static void wl_registry_global(WaylandServerClient *self, uint32_t id,
                               uint32_t name, const char *interface,
                               uint32_t version) {
  uint8_t payload[4 + string_length(interface) + 4];
  size_t offset = 0;
  offset = set_uint(payload, offset, name);
  offset = set_string(payload, offset, interface);
  offset = set_uint(payload, offset, version);
  wayland_server_client_send_event(self, id, 0, payload, sizeof(payload));
}

static void wl_registry_global_remove(WaylandServerClient *self, uint32_t id,
                                      uint32_t name) {
  uint8_t payload[4];
  set_uint(payload, 0, name);
  wayland_server_client_send_event(self, id, 1, payload, sizeof(payload));
}

static void wl_display_sync(WaylandServerClient *self,
                            WaylandPayloadDecoder *decoder) {
  uint32_t callback = wayland_payload_decoder_read_uint(decoder);
  if (!wayland_payload_decoder_finish(decoder)) {
    return;
  }

  printf("wl_display::sync %d\n", callback);

  wayland_server_client_add_object(self, callback, wl_callback_request_cb,
                                   self);

  wl_callback_done(self, callback, 0);
}

static void wl_display_get_registry(WaylandServerClient *self,
                                    WaylandPayloadDecoder *decoder) {
  uint32_t id = wayland_payload_decoder_read_uint(decoder);
  if (!wayland_payload_decoder_finish(decoder)) {
    return;
  }

  printf("wl_display::get_registry %d\n", id);

  wayland_server_client_add_object(self, id, wl_registry_request_cb, self);
  wl_registry_global(self, id, 1, "wl_compositor", 6);
  wl_registry_global(self, id, 2, "wl_shm", 2);
  wl_registry_global(self, id, 3, "wl_data_device_manager", 3);
  wl_registry_global(self, id, 4, "xdg_wm_base", 6);
}

static void wl_display_request_cb(uint16_t code, WaylandPayloadDecoder *decoder,
                                  void *user_data) {
  WaylandServerClient *self = user_data;

  switch (code) {
  case 0:
    wl_display_sync(self, decoder);
    break;
  case 1:
    wl_display_get_registry(self, decoder);
    break;
  default:
    break;
  }
}

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

  wayland_server_client_add_object(self, WL_DISPLAY_ID, wl_display_request_cb,
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
                                      uint16_t code, const uint8_t *payload,
                                      uint16_t payload_length) {
  // FIXME: Check may payload length (65535-8)
  uint32_t header[2];
  header[0] = id;
  header[1] = (payload_length + 8) << 16 | code;
  // FIXME: Handle partial writes
  write(self->fd, header, 8);
  write(self->fd, payload, payload_length);
}

void wayland_server_client_send_error(WaylandServerClient *self, uint32_t id,
                                      uint16_t code, const char *message) {
  uint8_t payload[1] = {0};
  uint16_t payload_length = 0;
  wayland_server_client_send_event(self, WL_DISPLAY_ID, 0, payload,
                                   payload_length);
}
