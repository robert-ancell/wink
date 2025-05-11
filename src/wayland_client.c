#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "wayland_client.h"

#include "socket_client.h"
#include "wayland_message_decoder.h"
#include "wl_callback_client.h"
#include "wl_compositor_client.h"
#include "wl_display_client.h"
#include "wl_registry_client.h"
#include "wl_shm_client.h"
#include "xdg_surface_client.h"
#include "xdg_toplevel_client.h"
#include "xdg_wm_base_client.h"

typedef struct {
  uint32_t id;
  WaylandClientEventCallback event_callback;
  WaylandClientDeleteCallback delete_callback;
  void *user_data;
} WaylandObject;

struct _WaylandClient {
  MainLoop *loop;
  SocketClient *socket;
  WaylandMessageDecoder *message_decoder;
  uint32_t next_id;
  WaylandObject *objects;
  size_t objects_length;
  WlDisplayClient *display;
  WlRegistryClient *registry;
  WlCompositorClient *compositor;
  WlShmClient *shm;
  XdgWmBaseClient *wm_base;
};

static uint32_t get_next_id(WaylandClient *self) { return self->next_id++; }

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

static void ping_cb(uint32_t serial, void *user_data) {
  WaylandClient *self = user_data;
  xdg_wm_base_client_pong(self->wm_base, serial);
}

static XdgWmBaseClientEventCallbacks wm_base_callbacks = {.ping = ping_cb};

static void format_cb(uint32_t format, void *user_data) {
  printf("SHM Format %d\n", format);
}

static WlShmClientEventCallbacks shm_callbacks = {.format = format_cb};

static void callback_done_cb(uint32_t callback_data, void *user_data) {
  // FIXME: Call callback provided in wayland_client_sync
  printf("DONE\n");
}

static WlCallbackClientEventCallbacks callback_callbacks = {
    .done = callback_done_cb};

static void error_cb(uint32_t object_id, uint32_t code, const char *message,
                     void *user_data) {
  printf("E: %d %d %s\n", object_id, code, message);
}

static void delete_id_cb(uint32_t id, void *user_data) {
  WaylandClient *self = user_data;

  WaylandObject *o = find_object(self, id);
  if (o == NULL) {
    return;
  }

  o->delete_callback(o->user_data);

  // FIXME: Binary search and reuse lookup from above.
  for (size_t i = 0; i < self->objects_length; i++) {
    if (o->id == id) {
      i++;
      while (i < self->objects_length) {
        self->objects[i - 1] = self->objects[i];
      }
      self->objects_length--;
    }
  }
}

static WlDisplayClientEventCallbacks display_callbacks = {
    .error = error_cb, .delete_id = delete_id_cb};

static void global_cb(uint32_t name, const char *interface, uint32_t version,
                      void *user_data) {
  WaylandClient *self = user_data;

  printf("%s %d\n", interface, version);
  if (strcmp(interface, "wl_compositor") == 0) {
    uint32_t compositor_id = get_next_id(self);
    self->compositor = wl_compositor_client_new(self, compositor_id);
    wl_registry_client_bind(self->registry, name, interface, version,
                            compositor_id);
  } else if (strcmp(interface, "wl_shm") == 0) {
    uint32_t shm_id = get_next_id(self);
    self->shm = wl_shm_client_new(self, shm_id, &shm_callbacks, self);
    wl_registry_client_bind(self->registry, name, interface, version, shm_id);
  } else if (strcmp(interface, "xdg_wm_base") == 0) {
    uint32_t wm_base_id = get_next_id(self);
    self->wm_base =
        xdg_wm_base_client_new(self, wm_base_id, &wm_base_callbacks, self);
    wl_registry_client_bind(self->registry, name, interface, version,
                            wm_base_id);
  }
}

static void global_remove_cb(uint32_t name, void *user_data) {}

static WlRegistryClientEventCallbacks registry_callbacks = {
    .global = global_cb, .global_remove = global_remove_cb};

static void message_cb(uint32_t id, uint16_t code,
                       WaylandPayloadDecoder *payload, void *user_data) {
  WaylandClient *self = user_data;

  WaylandObject *o = find_object(self, id);
  if (o == NULL) {
    // FIXME: Generate error
    printf("unknown object %d\n", id);
    return;
  }

  o->event_callback(code, payload, o->user_data);
}

static void read_cb(void *user_data) {
  WaylandClient *self = user_data;

  uint8_t data[1024];
  ssize_t data_length = read(socket_client_get_fd(self->socket), data, 1024);
  if (data_length == -1) {
    return;
  }

  wayland_message_decoder_write(self->message_decoder, data, data_length);
}

WaylandClient *wayland_client_new(MainLoop *loop) {
  WaylandClient *self = malloc(sizeof(WaylandClient));
  self->loop = main_loop_ref(loop);
  self->socket = socket_client_new();
  self->message_decoder = wayland_message_decoder_new(message_cb, self);
  self->next_id = 1;
  self->objects = NULL;
  self->objects_length = 0;
  self->display = NULL;
  self->registry = NULL;
  self->compositor = NULL;
  self->shm = NULL;
  self->wm_base = NULL;
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
                               WaylandClientDeleteCallback delete_callback,
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
  o->delete_callback = delete_callback;
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
  assert(write(fd, header, 8) == 8);
  assert(write(fd, wayland_payload_encoder_get_data(encoder), payload_length) ==
         payload_length);
}

void wayland_client_sync(WaylandClient *self,
                         WaylandClientSyncDoneCallback callback,
                         void *user_data) {
  uint32_t callback_id = get_next_id(self);
  wl_callback_client_new(self, callback_id, &callback_callbacks, self);
  wl_display_client_sync(self->display, callback_id);
}
