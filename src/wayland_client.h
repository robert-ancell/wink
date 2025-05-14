typedef struct _WaylandClient WaylandClient;

#pragma once

#include <stdbool.h>

#include "main_loop.h"
#include "wayland_message_decoder.h"
#include "wayland_message_encoder.h"
#include "wl_compositor_client.h"
#include "wl_shm_client.h"
#include "xdg_wm_base_client.h"

typedef void (*WaylandClientConnectedCallback)(WaylandClient *self,
                                               void *user_data);

typedef void (*WaylandClientCloseCallback)(WaylandClient *self,
                                           void *user_data);

typedef void (*WaylandClientEventCallback)(WaylandClient *self,
                                           WaylandMessageDecoder *payload,
                                           void *user_data);

typedef void (*WaylandClientDeleteCallback)(WaylandClient *self,
                                            void *user_data);

typedef void (*WaylandClientSyncDoneCallback)(WaylandClient *self,
                                              uint32_t callback_data,
                                              void *user_data);

WaylandClient *
wayland_client_new(MainLoop *loop,
                   WaylandClientConnectedCallback connected_callback,
                   WaylandClientCloseCallback close_callback, void *user_data,
                   void (*user_data_unref)(void *));

WaylandClient *wayland_client_ref(WaylandClient *self);

void wayland_client_unref(WaylandClient *self);

bool wayland_client_connect(WaylandClient *self, const char *display);

uint32_t wayland_client_add_object(WaylandClient *self,
                                   WaylandClientEventCallback event_callback,
                                   WaylandClientDeleteCallback delete_callback,
                                   void *user_data,
                                   void (*user_data_unref)(void *));

void wayland_client_send_message(WaylandClient *self,
                                 WaylandMessageEncoder *message);

void wayland_client_sync(WaylandClient *self,
                         WaylandClientSyncDoneCallback done_callback,
                         void *user_data, void (*user_data_unref)(void *));

WlCompositorClient *wayland_client_get_compositor(WaylandClient *self);

WlShmClient *wayland_client_get_shm(WaylandClient *self);

XdgWmBaseClient *wayland_client_get_wm_base(WaylandClient *self);
