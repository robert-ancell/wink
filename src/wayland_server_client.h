typedef struct _WaylandServerClient WaylandServerClient;

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "main_loop.h"
#include "wayland_message_decoder.h"
#include "wayland_message_encoder.h"

typedef void (*WaylandServerClientDisconnectCallback)(WaylandServerClient *self,
                                                      void *user_data);

typedef void (*WaylandServerClientRequestCallback)(
    WaylandServerClient *self, WaylandMessageDecoder *decoder, void *user_data);

WaylandServerClient *wayland_server_client_new(
    MainLoop *loop, Fd *fd,
    WaylandServerClientDisconnectCallback disconnect_callback, void *user_data,
    void (*user_data_unref)(void *));

WaylandServerClient *wayland_server_client_ref(WaylandServerClient *self);

void wayland_server_client_unref(WaylandServerClient *self);

void wayland_server_client_add_object(
    WaylandServerClient *self, uint32_t id,
    WaylandServerClientRequestCallback request_callback, void *user_data,
    void (*user_data_unref)(void *));

void wayland_server_client_send_message(WaylandServerClient *self,
                                        WaylandMessageEncoder *message);
