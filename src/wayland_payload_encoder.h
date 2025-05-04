#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "main_loop.h"

typedef struct _WaylandPayloadEncoder WaylandPayloadEncoder;

WaylandPayloadEncoder *wayland_payload_encoder_new();

WaylandPayloadEncoder *wayland_payload_encoder_ref(WaylandPayloadEncoder *self);

void wayland_payload_encoder_unref(WaylandPayloadEncoder *self);

void wayland_payload_encoder_write_int(WaylandPayloadEncoder *self,
                                       int32_t value);

void wayland_payload_encoder_write_uint(WaylandPayloadEncoder *self,
                                        uint32_t value);

void wayland_payload_encoder_write_string(WaylandPayloadEncoder *self,
                                          const char *value);

void wayland_payload_encoder_write_object(WaylandPayloadEncoder *self,
                                          uint32_t object);

void wayland_payload_encoder_write_new_id(WaylandPayloadEncoder *self,
                                          uint32_t new_id);

void wayland_payload_encoder_write_array(WaylandPayloadEncoder *self,
                                         const uint32_t *array);

void wayland_payload_encoder_write_fd(WaylandPayloadEncoder *self, int fd);

bool wayland_payload_encoder_finish(WaylandPayloadEncoder *self);

const uint8_t *wayland_payload_encoder_get_data(WaylandPayloadEncoder *self);

size_t wayland_payload_encoder_get_length(WaylandPayloadEncoder *self);
