#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "wayland_server_client.h"

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

static void wl_registry_request_cb(uint16_t code, const uint8_t *payload,
                                   uint16_t payload_length, void *user_data) {}

static void wl_display_sync(WaylandServerClient *self, const uint8_t *payload,
                            uint16_t payload_length) {
  if (payload_length != 4) {
    return;
  }

  uint32_t callback = ((uint32_t *)payload)[0];

  // FIXME
}

static void wl_display_get_registry(WaylandServerClient *self,
                                    const uint8_t *payload,
                                    uint16_t payload_length) {
  if (payload_length != 4) {
    return;
  }

  uint32_t id = ((uint32_t *)payload)[0];

  wayland_server_client_add_object(self, id, wl_registry_request_cb, self);
}

static void wl_display_request_cb(uint16_t code, const uint8_t *payload,
                                  uint16_t payload_length, void *user_data) {
  WaylandServerClient *self = user_data;

  switch (code) {
  case 0:
    wl_display_sync(self, payload, payload_length);
    return;
  case 1:
    wl_display_get_registry(self, payload, payload_length);
    return;
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
  uint32_t header[2];
  header[0] = id;
  header[1] = payload_length << 16 | code;
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
