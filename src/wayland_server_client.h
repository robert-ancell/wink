#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "main_loop.h"
#include "wayland_message_decoder.h"
#include "wayland_message_encoder.h"

typedef struct _WaylandServerClient WaylandServerClient;

typedef void (*WaylandServerClientRequestCallback)(
    WaylandMessageDecoder *decoder, void *user_data);

WaylandServerClient *wayland_server_client_new(MainLoop *loop, int fd);

WaylandServerClient *wayland_server_client_ref(WaylandServerClient *self);

void wayland_server_client_unref(WaylandServerClient *self);

void wayland_server_client_add_object(
    WaylandServerClient *self, uint32_t id,
    WaylandServerClientRequestCallback request_callback, void *user_data);

void wayland_server_client_send_message(WaylandServerClient *self,
                                        WaylandMessageEncoder *message);
