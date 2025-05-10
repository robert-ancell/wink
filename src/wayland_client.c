#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "wayland_client.h"

#include "socket_client.h"
#include "wl_display_client.h"
#include "wl_registry_client.h"
#include "wl_shm_client.h"

typedef struct {
  uint32_t id;
  WaylandClientEventCallback event_callback;
  void *user_data;
} WaylandObject;

struct _WaylandClient {
  MainLoop *loop;
  SocketClient *socket;
  uint32_t next_id;
  WaylandObject *objects;
  size_t objects_length;
  WlDisplayClient *display;
  WlRegistryClient *registry;
  WlShmClient *shm;
};

static uint32_t get_next_id(WaylandClient *self) { return self->next_id++; }

static void format_cb(uint32_t format, void *user_data) {
  printf("SHM Format %d\n", format);
}

static WlShmClientEventCallbacks shm_callbacks = {.format = format_cb};

static void error_cb(uint32_t object_id, uint32_t code, const char *message,
                     void *user_data) {
  printf("E: %d %d %s\n", object_id, code, message);
}

static void delete_id_cb(uint32_t id, void *user_data) {
  printf("DELETE %d\n", id);
}

static WlDisplayClientEventCallbacks display_callbacks = {
    .error = error_cb, .delete_id = delete_id_cb};

static void global_cb(uint32_t name, const char *interface, uint32_t version,
                      void *user_data) {
  WaylandClient *self = user_data;

  printf("%s %d\n", interface, version);
  if (strcmp(interface, "wl_shm") == 0) {
    uint32_t shm_id = get_next_id(self);
    self->shm = wl_shm_client_new(self, shm_id, &shm_callbacks, self);
    wl_registry_client_bind(self->registry, name, interface, version, shm_id);
  }
}

static void global_remove_cb(uint32_t name, void *user_data) {}

static WlRegistryClientEventCallbacks registry_callbacks = {
    .global = global_cb, .global_remove = global_remove_cb};

static WaylandObject *find_object(WaylandClient *self, uint32_t id) {
  // FIXME: Binary search
  for (size_t i = 0; i < self->objects_length; i++) {
    WaylandObject *o = &self->objects[i];
    if (o->id == id) {
      return o;
    }
  }

  return NULL;
}

static void decode_event(WaylandClient *self, uint32_t id, uint16_t code,
                         WaylandPayloadDecoder *decoder) {
  WaylandObject *o = find_object(self, id);
  if (o == NULL) {
    // FIXME: Generate error
    printf("unknown object %d\n", id);
    return;
  }

  o->event_callback(code, decoder, o->user_data);
}

static void read_cb(void *user_data) {
  WaylandClient *self = user_data;

  uint8_t data[1024];
  ssize_t data_length = read(socket_client_get_fd(self->socket), data, 1024);
  if (data_length == -1) {
    return;
  }

  // FIXME: Make WaylandMessageDecoder shared with both WaylandClient and
  // WaylandServerClient
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
    decode_event(self, id, code, decoder);
    wayland_payload_decoder_unref(decoder);

    offset += length;
  }
}

WaylandClient *wayland_client_new(MainLoop *loop) {
  WaylandClient *self = malloc(sizeof(WaylandClient));
  self->loop = main_loop_ref(loop);
  self->socket = socket_client_new();
  self->next_id = 1;
  self->objects = NULL;
  self->objects_length = 0;
  self->display = NULL;
  self->registry = NULL;
  self->shm = NULL;
  return self;
}

WaylandClient *wayland_client_ref(WaylandClient *self) {
  // FIXME
  return self;
}

void wayland_client_unref(WaylandClient *self) {
  // FIXME
}

bool wayland_client_connect(WaylandClient *self, const char *display) {
  if (display == NULL) {
    display = getenv("WAYLAND_DISPLAY");
  }
  if (display == NULL) {
    display = "wayland-0";
  }

  char path[1024];
  if (display[0] == '/') {
    snprintf(path, 1024, "%s", display);
  } else {
    const char *runtime_dir = getenv("XDG_RUNTIME_DIR");
    snprintf(path, 1024, "%s/%s", runtime_dir, display);
  }

  if (!socket_client_connect(self->socket, path)) {
    return false;
  }

  main_loop_add_fd(self->loop, socket_client_get_fd(self->socket), read_cb,
                   self);

  self->display =
      wl_display_client_new(self, get_next_id(self), &display_callbacks, self);
  uint32_t registry_id = get_next_id(self);
  self->registry =
      wl_registry_client_new(self, registry_id, &registry_callbacks, self);
  wl_display_client_get_registry(self->display, registry_id);

  return true;
}

void wayland_client_add_object(WaylandClient *self, uint32_t id,
                               WaylandClientEventCallback event_callback,
                               void *user_data) {
  if (find_object(self, id) != NULL) {
    // FIXME: error
    return;
  }

  self->objects_length++;
  self->objects =
      realloc(self->objects, sizeof(WaylandObject) * self->objects_length);
  WaylandObject *o = &self->objects[self->objects_length - 1];
  o->id = id;
  o->event_callback = event_callback;
  o->user_data = user_data;
}

void wayland_client_send_request(WaylandClient *self, uint32_t id,
                                 uint16_t code,
                                 WaylandPayloadEncoder *encoder) {
  size_t payload_length = wayland_payload_encoder_get_length(encoder);
  // FIXME: Check may payload length (65535-8)
  uint32_t header[2];
  header[0] = id;
  header[1] = (payload_length + 8) << 16 | code;
  // FIXME: Handle partial writes
  int fd = socket_client_get_fd(self->socket);
  write(fd, header, 8);
  write(fd, wayland_payload_encoder_get_data(encoder), payload_length);
}
