#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "main_loop.h"
#include "wayland_payload_decoder.h"

typedef struct _WaylandServerClient WaylandServerClient;

typedef void (*WaylandServerClientRequestCallback)(
    uint16_t code, WaylandPayloadDecoder *decoder, void *user_data);

WaylandServerClient *wayland_server_client_new(MainLoop *loop, int fd);

WaylandServerClient *wayland_server_client_ref(WaylandServerClient *self);

void wayland_server_client_unref(WaylandServerClient *self);

void wayland_server_client_add_object(
    WaylandServerClient *self, uint32_t id,
    WaylandServerClientRequestCallback request_callback, void *user_data);

void wayland_server_client_send_event(WaylandServerClient *self, uint32_t id,
                                      uint16_t code, const uint8_t *payload,
                                      uint16_t payload_length);

void wayland_server_client_send_error(WaylandServerClient *self, uint32_t id,
                                      uint16_t code, const char *message);
