#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wayland_client.h"

#include "ref.h"
#include "socket_client.h"
#include "wayland_stream_decoder.h"
#include "wayland_stream_encoder.h"
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
  void (*user_data_unref)(void *);
} WaylandObject;

struct _WaylandClient {
  ref_t ref;
  WaylandClientConnectedCallback connected_callback;
  WaylandClientCloseCallback close_callback;
  void *user_data;
  void (*user_data_unref)(void *);
  MainLoop *loop;
  SocketClient *socket;
  WaylandStreamDecoder *stream_decoder;
  WaylandStreamEncoder *stream_encoder;
  uint32_t next_id;
  WaylandObject *objects;
  size_t objects_length;
  WlDisplayClient *display;
  WlRegistryClient *registry;
  WlCompositorClient *compositor;
  WlShmClient *shm;
  XdgWmBaseClient *wm_base;
};

typedef struct {
  WaylandClient *self;
  WaylandClientSyncDoneCallback done_callback;
  void *user_data;
  void (*user_data_unref)(void *);
} CallbackData;

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

static void format_cb(uint32_t format, void *user_data) {}

static WlShmClientEventCallbacks shm_callbacks = {.format = format_cb};

static void callback_done_cb(uint32_t callback_data, void *user_data) {
  CallbackData *data = user_data;
  data->done_callback(data->self, callback_data, data->user_data);
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

  if (o->delete_callback) {
    o->delete_callback(self, o->user_data);
  }

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
  if (o->user_data_unref) {
    o->user_data_unref(o->user_data);
  }
  free(o);
}

static WlDisplayClientEventCallbacks display_callbacks = {
    .error = error_cb, .delete_id = delete_id_cb};

static void global_cb(uint32_t name, const char *interface, uint32_t version,
                      void *user_data) {
  WaylandClient *self = user_data;

  if (strcmp(interface, "wl_compositor") == 0) {
    self->compositor = wl_compositor_client_new(self);
    wl_registry_client_bind(self->registry, name, interface, version,
                            wl_compositor_client_get_id(self->compositor));
  } else if (strcmp(interface, "wl_shm") == 0) {
    self->shm = wl_shm_client_new(self, &shm_callbacks, self, NULL);
    wl_registry_client_bind(self->registry, name, interface, version,
                            wl_shm_client_get_id(self->shm));
  } else if (strcmp(interface, "xdg_wm_base") == 0) {
    self->wm_base =
        xdg_wm_base_client_new(self, &wm_base_callbacks, self, NULL);
    wl_registry_client_bind(self->registry, name, interface, version,
                            xdg_wm_base_client_get_id(self->wm_base));
  }
}

static void global_remove_cb(uint32_t name, void *user_data) {}

static WlRegistryClientEventCallbacks registry_callbacks = {
    .global = global_cb, .global_remove = global_remove_cb};

static void message_cb(WaylandStreamDecoder *decoder,
                       WaylandMessageDecoder *message, void *user_data) {
  WaylandClient *self = user_data;

  uint32_t id = wayland_message_decoder_get_id(message);
  WaylandObject *o = find_object(self, id);
  if (o == NULL) {
    // FIXME: Generate error
    printf("unknown object %d\n", id);
    return;
  }

  o->event_callback(self, message, o->user_data);
}

static void close_cb(WaylandStreamDecoder *decoder, void *user_data) {
  WaylandClient *self = user_data;
  self->close_callback(self, self->user_data);
}

static void registry_done_cb(WaylandClient *self, uint32_t callback_data,
                             void *user_data) {
  self->connected_callback(self, self->user_data);
}

WaylandClient *
wayland_client_new(MainLoop *loop,
                   WaylandClientConnectedCallback connected_callback,
                   WaylandClientCloseCallback close_callback, void *user_data,
                   void (*user_data_unref)(void *)) {
  WaylandClient *self = malloc(sizeof(WaylandClient));
  ref_init(&self->ref);
  self->connected_callback = connected_callback;
  self->close_callback = close_callback;
  self->user_data = user_data;
  self->user_data_unref = user_data_unref;
  self->loop = main_loop_ref(loop);
  self->socket = socket_client_new();
  self->stream_encoder = NULL;
  self->stream_decoder = NULL;
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
  ref_inc(&self->ref);
  return self;
}

void wayland_client_unref(WaylandClient *self) {
  if (ref_dec(&self->ref)) {
    if (self->user_data_unref) {
      self->user_data_unref(self->user_data);
    }
    main_loop_unref(self->loop);
    socket_client_unref(self->socket);
    if (self->stream_decoder) {
      wayland_stream_decoder_unref(self->stream_decoder);
    }
    if (self->stream_encoder) {
      wayland_stream_encoder_unref(self->stream_encoder);
    }
    for (size_t i = 0; i < self->objects_length; i++) {
      WaylandObject *o = &self->objects[i];
      if (o->user_data_unref) {
        o->user_data_unref(o->user_data);
      }
    }
    free(self->objects);
    if (self->display) {
      wl_display_client_unref(self->display);
    }
    if (self->registry) {
      wl_registry_client_unref(self->registry);
    }
    if (self->compositor) {
      wl_compositor_client_unref(self->compositor);
    }
    if (self->shm) {
      wl_shm_client_unref(self->shm);
    }
    if (self->wm_base) {
      xdg_wm_base_client_unref(self->wm_base);
    }
    free(self);
  }
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

  Fd *fd = socket_client_get_fd(self->socket);
  self->stream_encoder = wayland_stream_encoder_new(fd);
  self->stream_decoder = wayland_stream_decoder_new(self->loop, fd, message_cb,
                                                    close_cb, self, NULL);

  self->display = wl_display_client_new(self, &display_callbacks, self, NULL);
  self->registry =
      wl_registry_client_new(self, &registry_callbacks, self, NULL);
  wl_display_client_get_registry(self->display,
                                 wl_registry_client_get_id(self->registry));
  wayland_client_sync(self, registry_done_cb, NULL, NULL);

  return true;
}

uint32_t wayland_client_add_object(WaylandClient *self,
                                   WaylandClientEventCallback event_callback,
                                   WaylandClientDeleteCallback delete_callback,
                                   void *user_data,
                                   void (*user_data_unref)(void *)) {
  self->objects_length++;
  self->objects =
      realloc(self->objects, sizeof(WaylandObject) * self->objects_length);
  WaylandObject *o = &self->objects[self->objects_length - 1];
  o->id = get_next_id(self);
  o->event_callback = event_callback;
  o->delete_callback = delete_callback;
  o->user_data = user_data;
  o->user_data_unref = user_data_unref;

  return o->id;
}

void wayland_client_send_message(WaylandClient *self,
                                 WaylandMessageEncoder *message) {
  wayland_stream_encoder_write(self->stream_encoder, message);
}

void wayland_client_sync(WaylandClient *self,
                         WaylandClientSyncDoneCallback done_callback,
                         void *user_data, void (*user_data_unref)(void *)) {

  CallbackData *data = malloc(sizeof(CallbackData));
  data->self = self;
  data->done_callback = done_callback;
  data->user_data = user_data;
  data->user_data_unref = user_data_unref;
  WlCallbackClient *callback =
      wl_callback_client_new(self, &callback_callbacks, data, free);
  wl_display_client_sync(self->display, wl_callback_client_get_id(callback));
}

WlCompositorClient *wayland_client_get_compositor(WaylandClient *self) {
  return self->compositor;
}

WlShmClient *wayland_client_get_shm(WaylandClient *self) { return self->shm; }

XdgWmBaseClient *wayland_client_get_wm_base(WaylandClient *self) {
  return self->wm_base;
}
