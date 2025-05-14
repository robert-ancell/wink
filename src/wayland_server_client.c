#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wayland_server_client.h"

#include "ref.h"
#include "wayland_stream_decoder.h"
#include "wayland_stream_encoder.h"
#include "wl_buffer_server.h"
#include "wl_callback_server.h"
#include "wl_compositor_server.h"
#include "wl_data_device_manager_server.h"
#include "wl_display_server.h"
#include "wl_registry_server.h"
#include "wl_shm_pool_server.h"
#include "wl_shm_server.h"
#include "wl_surface_server.h"
#include "xdg_surface_server.h"
#include "xdg_toplevel_server.h"
#include "xdg_wm_base_server.h"

#define WL_DISPLAY_ID 1

typedef struct {
  uint32_t id;
  WaylandServerClientRequestCallback request_callback;
  void *user_data;
  void (*user_data_unref)(void *);
} WaylandObject;

struct _WaylandServerClient {
  ref_t ref;
  WaylandStreamDecoder *stream_decoder;
  WaylandStreamEncoder *stream_encoder;
  WaylandObject *objects;
  size_t objects_length;
};

static void xdg_toplevel_destroy(void *user_data) {
  printf("xdg_toplevel::destroy\n");
}

static void xdg_toplevel_set_parent(uint32_t parent, void *user_data) {
  printf("xdg_toplevel::set_parent %d\n", parent);
}

static void xdg_toplevel_set_title(const char *title, void *user_data) {
  printf("xdg_toplevel::set_title '%s'\n", title);
}

static void xdg_toplevel_set_app_id(const char *app_id, void *user_data) {
  printf("xdg_toplevel::set_app_id '%s'\n", app_id);
}

static void xdg_toplevel_show_window_menu(uint32_t seat, uint32_t serial,
                                          int32_t x, int32_t y,
                                          void *user_data) {
  printf("xdg_toplevel::show_window_menu\n");
}

static void xdg_toplevel_move(uint32_t seat, uint32_t serial, void *user_data) {
  printf("xdg_toplevel::move\n");
}

static void xdg_toplevel_resize(uint32_t seat, uint32_t serial, uint32_t edges,
                                void *user_data) {
  printf("xdg_toplevel::resize\n");
}

static void xdg_toplevel_set_max_size(int32_t width, int32_t height,
                                      void *user_data) {
  printf("xdg_toplevel::set_max_size %dx%d\n", width, height);
}

static void xdg_toplevel_set_min_size(int32_t width, int32_t height,
                                      void *user_data) {
  printf("xdg_toplevel::set_min_size %dx%d\n", width, height);
}

static void xdg_toplevel_set_maximized(void *user_data) {
  printf("xdg_toplevel::set_maximized\n");
}

static void xdg_toplevel_unset_maximized(void *user_data) {
  printf("xdg_toplevel::unset_maximized\n");
}

static void xdg_toplevel_set_fullscreen(uint32_t output, void *user_data) {
  printf("xdg_toplevel::set_fullscreen %d\n", output);
}

static void xdg_toplevel_unset_fullscreen(void *user_data) {
  printf("xdg_toplevel::unset_fullscreen\n");
}

static void xdg_toplevel_set_minimized(void *user_data) {
  printf("xdg_toplevel::set_minimized\n");
}

static XdgToplevelServerRequestCallbacks xdg_toplevel_request_callbacks = {
    .destroy = xdg_toplevel_destroy,
    .set_parent = xdg_toplevel_set_parent,
    .set_title = xdg_toplevel_set_title,
    .set_app_id = xdg_toplevel_set_app_id,
    .show_window_menu = xdg_toplevel_show_window_menu,
    .move = xdg_toplevel_move,
    .resize = xdg_toplevel_resize,
    .set_max_size = xdg_toplevel_set_max_size,
    .set_min_size = xdg_toplevel_set_min_size,
    .set_maximized = xdg_toplevel_set_maximized,
    .unset_maximized = xdg_toplevel_unset_maximized,
    .set_fullscreen = xdg_toplevel_set_fullscreen,
    .unset_fullscreen = xdg_toplevel_unset_fullscreen,
    .set_minimized = xdg_toplevel_set_minimized};

static void xdg_surface_destroy(void *user_data) {
  printf("xdg_surface::destroy\n");
}

static void xdg_surface_get_toplevel(uint32_t id, void *user_data) {
  WaylandServerClient *self = user_data;

  printf("xdg_surface::get_toplevel %d\n", id);

  xdg_toplevel_server_new(self, id, &xdg_toplevel_request_callbacks, self,
                          NULL);
}

static void xdg_surface_get_popup(uint32_t id, uint32_t parent,
                                  uint32_t positioner, void *user_data) {
  printf("xdg_surface::get_popup\n");
}

static void xdg_surface_set_window_geometry(int32_t x, int32_t y, int32_t width,
                                            int32_t height, void *user_data) {
  printf("xdg_surface::set_window_geometry\n");
}

static void xdg_surface_ack_configure(uint32_t serial, void *user_data) {
  printf("xdg_surface::ack_configure\n");
}

static XdgSurfaceServerRequestCallbacks xdg_surface_request_callbacks = {
    .destroy = xdg_surface_destroy,
    .get_toplevel = xdg_surface_get_toplevel,
    .get_popup = xdg_surface_get_popup,
    .set_window_geometry = xdg_surface_set_window_geometry,
    .ack_configure = xdg_surface_ack_configure};

static void wl_surface_destroy(void *user_data) {
  printf("wl_surface::destroy\n");
}

static void wl_surface_attach(uint32_t buffer, int32_t x, int32_t y,
                              void *user_data) {
  printf("wl_surface::attach\n");
}

static void wl_surface_damage(int32_t x, int32_t y, int32_t width,
                              int32_t height, void *user_data) {
  printf("wl_surface::damage\n");
}

static void wl_surface_frame(uint32_t callback, void *user_data) {
  printf("wl_surface::frame\n");
}

static void wl_surface_set_opaque_region(uint32_t region, void *user_data) {
  printf("wl_surface::set_opaque_region\n");
}

static void wl_surface_set_input_region(uint32_t region, void *user_data) {
  printf("wl_surface::set_input_region\n");
}

static void wl_surface_commit(void *user_data) {
  printf("wl_surface::commit\n");
}

static void wl_surface_set_buffer_transform(int32_t transform,
                                            void *user_data) {
  printf("wl_surface::set_buffer_transform\n");
}

static void wl_surface_set_buffer_scale(int32_t scale, void *user_data) {
  printf("wl_surface::set_buffer_scale\n");
}

static void wl_surface_damage_buffer(int32_t x, int32_t y, int32_t width,
                                     int32_t height, void *user_data) {
  printf("wl_surface::damage_buffer\n");
}

static void wl_surface_offset(int32_t x, int32_t y, void *user_data) {
  printf("wl_surface::offset\n");
}

static WlSurfaceServerRequestCallbacks wl_surface_request_callbacks = {
    .destroy = wl_surface_destroy,
    .attach = wl_surface_attach,
    .damage = wl_surface_damage,
    .frame = wl_surface_frame,
    .set_opaque_region = wl_surface_set_opaque_region,
    .set_input_region = wl_surface_set_input_region,
    .commit = wl_surface_commit,
    .set_buffer_transform = wl_surface_set_buffer_transform,
    .set_buffer_scale = wl_surface_set_buffer_scale,
    .damage_buffer = wl_surface_damage_buffer,
    .offset = wl_surface_offset};

static void wl_compositor_create_surface(uint32_t id, void *user_data) {
  WaylandServerClient *self = user_data;

  printf("wl_compositor::create_surface %d\n", id);

  wl_surface_server_new(self, id, &wl_surface_request_callbacks, self, NULL);
}

static void wl_compositor_create_region(uint32_t id, void *user_data) {
  printf("wl_compositor::create_region %d\n", id);
}

static WlCompositorServerRequestCallbacks wl_compositor_request_callbacks = {
    .create_surface = wl_compositor_create_surface,
    .create_region = wl_compositor_create_region};

static void wl_buffer_destroy(void *user_data) {
  printf("wl_buffer::destroy\n");
}

static WlBufferServerRequestCallbacks wl_buffer_request_callbacks = {
    .destroy = wl_buffer_destroy};

static void wl_shm_pool_create_buffer(uint32_t id, int32_t offset,
                                      int32_t width, int32_t height,
                                      int32_t stride, uint32_t format,
                                      void *user_data) {
  WaylandServerClient *self = user_data;

  printf("wl_shm_pool::create_buffer\n");

  wl_buffer_server_new(self, id, &wl_buffer_request_callbacks, self, NULL);
}

static void wl_shm_pool_destroy(void *user_data) {
  printf("wl_shm_pool::destroy\n");
}

static void wl_shm_pool_resize(int32_t size, void *user_data) {
  printf("wl_shm_pool::resize %d\n", size);
}

static WlShmPoolServerRequestCallbacks wl_shm_pool_request_callbacks = {
    .create_buffer = wl_shm_pool_create_buffer,
    .destroy = wl_shm_pool_destroy,
    .resize = wl_shm_pool_resize};

static void wl_shm_create_pool(uint32_t id, int fd, int32_t size,
                               void *user_data) {
  WaylandServerClient *self = user_data;

  printf("wl_shm::create_pool %d\n", id);

  wl_shm_pool_server_new(self, id, &wl_shm_pool_request_callbacks, self, NULL);
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
  WaylandServerClient *self = user_data;

  printf("xdg_wm_base::get_xdg_surface %d %d\n", id, surface);

  xdg_surface_server_new(self, id, &xdg_surface_request_callbacks, self, NULL);
}

static void xdg_wm_base_pong(uint32_t serial, void *user_data) {
  printf("xdg_wm_base::pong %d\n", serial);
}

static XdgWmBaseServerRequestCallbacks xdg_wm_base_request_callbacks = {
    .destroy = xdg_wm_base_destroy,
    .create_positioner = xdg_wm_base_create_positioner,
    .get_xdg_surface = xdg_wm_base_get_xdg_surface,
    .pong = xdg_wm_base_pong};

static void wl_registry_bind(uint32_t name, const char *id_interface,
                             uint32_t id_version, uint32_t id,
                             void *user_data) {
  WaylandServerClient *self = user_data;

  printf("wl_registry::bind %d %s %d %d\n", name, id_interface, id_version, id);

  switch (name) {
  case 1:
    // FIXME: Store object
    wl_compositor_server_new(self, id, &wl_compositor_request_callbacks, self,
                             NULL);
    break;
  case 2:
    // FIXME: Store object
    wl_shm_server_new(self, id, &wl_shm_request_callbacks, self, NULL);
    break;
  case 3:
    // FIXME: Store object
    wl_data_device_manager_server_new(
        self, id, &wl_data_device_manager_request_callbacks, self, NULL);
    break;
  case 4:
    // FIXME: Store object
    xdg_wm_base_server_new(self, id, &xdg_wm_base_request_callbacks, self,
                           NULL);
    break;
  }
}

static WlRegistryServerRequestCallbacks wl_registry_request_callbacks = {
    .bind = wl_registry_bind};

static void wl_display_sync(uint32_t callback, void *user_data) {
  WaylandServerClient *self = user_data;

  printf("wl_display::sync %d\n", callback);

  WlCallbackServer *object = wl_callback_server_new(self, callback);

  wl_callback_server_done(object, 0);
}

static void wl_display_get_registry(uint32_t id, void *user_data) {
  WaylandServerClient *self = user_data;

  printf("wl_display::get_registry %d\n", id);

  WlRegistryServer *registry = wl_registry_server_new(
      self, id, &wl_registry_request_callbacks, self, NULL);
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

static void message_cb(WaylandMessageDecoder *message, void *user_data) {
  WaylandServerClient *self = user_data;

  uint32_t id = wayland_message_decoder_get_id(message);
  WaylandObject *o = find_object(self, id);
  if (o == NULL) {
    // FIXME: Generate error
    printf("unknown object %d\n", id);
    return;
  }

  o->request_callback(message, o->user_data);
}

WaylandServerClient *wayland_server_client_new(MainLoop *loop, int fd) {
  WaylandServerClient *self = malloc(sizeof(WaylandServerClient));
  ref_init(&self->ref);
  self->stream_decoder =
      wayland_stream_decoder_new(loop, fd, message_cb, self, NULL);
  self->stream_encoder = wayland_stream_encoder_new(fd);
  self->objects = NULL;
  self->objects_length = 0;

  wl_display_server_new(self, WL_DISPLAY_ID, &wl_display_request_callbacks,
                        self, NULL);

  return self;
}

WaylandServerClient *wayland_server_client_ref(WaylandServerClient *self) {
  ref_inc(&self->ref);
  return self;
}

void wayland_server_client_unref(WaylandServerClient *self) {
  if (ref_dec(&self->ref)) {
    wayland_stream_decoder_unref(self->stream_decoder);
    wayland_stream_encoder_unref(self->stream_encoder);
    for (size_t i = 0; i < self->objects_length; i++) {
      WaylandObject *o = &self->objects[i];
      if (o->user_data_unref) {
        o->user_data_unref(o->user_data);
      }
    }
    free(self->objects);
    free(self);
  }
}

void wayland_server_client_add_object(
    WaylandServerClient *self, uint32_t id,
    WaylandServerClientRequestCallback request_callback, void *user_data,
    void (*user_data_unref)(void *)) {
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
  o->user_data_unref = user_data_unref;
}

void wayland_server_client_send_message(WaylandServerClient *self,
                                        WaylandMessageEncoder *message) {
  wayland_stream_encoder_write(self->stream_encoder, message);
}
