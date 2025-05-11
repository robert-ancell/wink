#pragma once

#include <stdbool.h>

#include "main_loop.h"
#include "wayland_payload_decoder.h"
#include "wayland_payload_encoder.h"

typedef struct _WaylandClient WaylandClient;

typedef void (*WaylandClientEventCallback)(uint16_t code,
                                           WaylandPayloadDecoder *payload,
                                           void *user_data);

typedef void (*WaylandClientSyncDoneCallback)(uint32_t callback_data,
                                              void *user_data);

WaylandClient *wayland_client_new(MainLoop *loop);

WaylandClient *wayland_client_ref(WaylandClient *self);

void wayland_client_unref(WaylandClient *self);

bool wayland_client_connect(WaylandClient *self, const char *display);

void wayland_client_add_object(WaylandClient *self, uint32_t id,
                               WaylandClientEventCallback event_callback,
                               void *user_data);

void wayland_client_send_request(WaylandClient *self, uint32_t id,
                                 uint16_t code, WaylandPayloadEncoder *encoder);

void wayland_client_sync(WaylandClient *self,
                         WaylandClientSyncDoneCallback callback,
                         void *user_data);
