#pragma once

#include <stdint.h>

#include "wayland_payload_decoder.h"
#include "wayland_server_client.h"

typedef struct {
  void (*destroy)(void *user_data);
  void (*set_parent)(uint32_t parent, void *user_data);
  void (*set_title)(const char *title, void *user_data);
  void (*set_app_id)(const char *app_id, void *user_data);
  void (*show_window_menu)(uint32_t seat, uint32_t serial, int32_t x, int32_t y,
                           void *user_data);
  void (*move)(uint32_t seat, uint32_t serial, void *user_data);
  void (*resize)(uint32_t seat, uint32_t serial, uint32_t edges,
                 void *user_data);
  void (*set_max_size)(int32_t width, int32_t height, void *user_data);
  void (*set_min_size)(int32_t width, int32_t height, void *user_data);
  void (*set_maximized)(void *user_data);
  void (*unset_maximized)(void *user_data);
  void (*set_fullscreen)(uint32_t output, void *user_data);
  void (*unset_fullscreen)(void *user_data);
  void (*set_minimized)(void *user_data);
} XdgToplevelServerRequestCallbacks;

typedef struct _XdgToplevelServer XdgToplevelServer;

XdgToplevelServer *xdg_toplevel_server_new(
    WaylandServerClient *client, uint32_t id,
    const XdgToplevelServerRequestCallbacks *request_callbacks,
    void *user_data);

XdgToplevelServer *xdg_toplevel_server_ref(XdgToplevelServer *self);

void xdg_toplevel_server_unref(XdgToplevelServer *self);

void xdg_toplevel_server_configure(XdgToplevelServer *self, int32_t width,
                                   int32_t height, uint32_t *states);

void xdg_toplevel_server_close(XdgToplevelServer *self);

void xdg_toplevel_server_configure_bounds(XdgToplevelServer *self,
                                          int32_t width, int32_t height);

void xdg_toplevel_server_wm_capabilities(XdgToplevelServer *self,
                                         uint32_t *capabilities);
