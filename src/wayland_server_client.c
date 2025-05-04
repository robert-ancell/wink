#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "wayland_server_client.h"
#include "wl_shm_server.h"

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

static size_t get_uint(const uint8_t *payload, size_t offset, uint32_t *value) {
  *value = *(uint32_t *)(payload + offset);
  return offset + 4;
}

static size_t get_string(const uint8_t *payload, size_t offset,
                         const char **value) {
  uint32_t length;
  offset = get_uint(payload, offset, &length);
  *value = (const char *)(payload + offset);
  size_t n_words = (length + 3) / 4;
  return offset + n_words * 4;
}

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

static void wl_compositor_create_surface(WaylandServerClient *self,
                                         const uint8_t *payload,
                                         uint16_t payload_length) {
  uint32_t id;
  size_t offset = 0;
  offset = get_uint(payload, offset, &id);

  printf("wl_compositor::create_surface %d\n", id);
}

static void wl_compositor_create_region(WaylandServerClient *self,
                                        const uint8_t *payload,
                                        uint16_t payload_length) {
  // FIXME
  printf("wl_compositor::create_region\n");
}

static void wl_compositor_request_cb(uint16_t code, const uint8_t *payload,
                                     uint16_t payload_length, void *user_data) {
  WaylandServerClient *self = user_data;

  switch (code) {
  case 0:
    wl_compositor_create_surface(self, payload, payload_length);
    break;
  case 1:
    wl_compositor_create_region(self, payload, payload_length);
    break;
  }
}

static void wl_shm_create_pool(void *user_data) {
  // FIXME
  printf("wl_shm::create_pool\n");
}

static void wl_shm_release(void *user_data) { printf("wl_shm::release\n"); }

static WlShmServerRequestCallbacks wl_shm_request_callbacks = {
    .create_pool = wl_shm_create_pool, .release = wl_shm_release};

static void wl_data_device_manager_request_cb(uint16_t code,
                                              const uint8_t *payload,
                                              uint16_t payload_length,
                                              void *user_data) {}

static void xdg_wm_base_destroy(WaylandServerClient *self,
                                const uint8_t *payload,
                                uint16_t payload_length) {
  printf("xdg_wm_base::destroy\n");
}

static void xdg_wm_base_create_positioner(WaylandServerClient *self,
                                          const uint8_t *payload,
                                          uint16_t payload_length) {
  printf("xdg_wm_base::create_positioner\n");
}

static void xdg_wm_base_get_xdg_surface(WaylandServerClient *self,
                                        const uint8_t *payload,
                                        uint16_t payload_length) {
  uint32_t id, surface;
  size_t offset = 0;
  offset = get_uint(payload, offset, &id);
  offset = get_uint(payload, offset, &surface);
  printf("xdg_wm_base::get_xdg_surface %d %d\n", id, surface);
}

static void xdg_wm_base_pong(WaylandServerClient *self, const uint8_t *payload,
                             uint16_t payload_length) {
  printf("xdg_wm_base::pong\n");
}

static void xdg_wm_base_request_cb(uint16_t code, const uint8_t *payload,
                                   uint16_t payload_length, void *user_data) {
  WaylandServerClient *self = user_data;

  switch (code) {
  case 0:
    xdg_wm_base_destroy(self, payload, payload_length);
    break;
  case 1:
    xdg_wm_base_create_positioner(self, payload, payload_length);
    break;
  case 2:
    xdg_wm_base_get_xdg_surface(self, payload, payload_length);
    break;
  case 3:
    xdg_wm_base_pong(self, payload, payload_length);
    break;
  }
}

static void wl_callback_request_cb(uint16_t code, const uint8_t *payload,
                                   uint16_t payload_length, void *user_data) {}

static void wl_callback_done(WaylandServerClient *self, uint32_t id,
                             uint32_t callback_data) {
  uint8_t payload[4];
  set_uint(payload, 0, callback_data);
  wayland_server_client_send_event(self, id, 0, payload, sizeof(payload));
}

static void wl_registry_bind(WaylandServerClient *self, const uint8_t *payload,
                             uint16_t payload_length) {
  // if (payload_length != 8)
  //  {
  // return;
  // }

  uint32_t name, version, id;
  const char *interface;
  size_t offset = 0;
  offset = get_uint(payload, offset, &name);
  offset = get_string(payload, offset, &interface);
  offset = get_uint(payload, offset, &version);
  offset = get_uint(payload, offset, &id);

  printf("wl_registry::bind %d %s %d %d\n", name, interface, version, id);

  switch (name) {
  case 1:
    wayland_server_client_add_object(self, id, wl_compositor_request_cb, self);
    break;
  case 2:
    // FIXME: Store object
    wl_shm_server_new(self, id, &wl_shm_request_callbacks, self);
    break;
  case 3:
    wayland_server_client_add_object(self, id,
                                     wl_data_device_manager_request_cb, self);
    break;
  case 4:
    wayland_server_client_add_object(self, id, xdg_wm_base_request_cb, self);
    break;
  }
}

static void wl_registry_request_cb(uint16_t code, const uint8_t *payload,
                                   uint16_t payload_length, void *user_data) {
  WaylandServerClient *self = user_data;

  switch (code) {
  case 0:
    wl_registry_bind(self, payload, payload_length);
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

static void wl_display_sync(WaylandServerClient *self, const uint8_t *payload,
                            uint16_t payload_length) {
  if (payload_length != 4) {
    return;
  }

  uint32_t callback;
  get_uint(payload, 0, &callback);

  printf("wl_display::sync %d\n", callback);

  wayland_server_client_add_object(self, callback, wl_callback_request_cb,
                                   self);

  wl_callback_done(self, callback, 0);
}

static void wl_display_get_registry(WaylandServerClient *self,
                                    const uint8_t *payload,
                                    uint16_t payload_length) {
  if (payload_length != 4) {
    return;
  }

  uint32_t id;
  get_uint(payload, 0, &id);

  printf("wl_display::get_registry %d\n", id);

  wayland_server_client_add_object(self, id, wl_registry_request_cb, self);
  wl_registry_global(self, id, 1, "wl_compositor", 6);
  wl_registry_global(self, id, 2, "wl_shm", 2);
  wl_registry_global(self, id, 3, "wl_data_device_manager", 3);
  wl_registry_global(self, id, 4, "xdg_wm_base", 6);
}

static void wl_display_request_cb(uint16_t code, const uint8_t *payload,
                                  uint16_t payload_length, void *user_data) {
  WaylandServerClient *self = user_data;

  switch (code) {
  case 0:
    wl_display_sync(self, payload, payload_length);
    break;
  case 1:
    wl_display_get_registry(self, payload, payload_length);
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
                           uint16_t code, const uint8_t *payload,
                           uint16_t payload_length) {
  printf("%d %d (%d)\n", id, code, payload_length);
  WaylandObject *o = find_object(self, id);
  if (o == NULL) {
    // FIXME: Generate error
    return;
  }

  o->request_callback(code, payload, payload_length, o->user_data);
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
    decode_request(self, id, code, payload, length - 8);

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
