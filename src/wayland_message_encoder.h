#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "main_loop.h"

typedef struct _WaylandMessageEncoder WaylandMessageEncoder;

WaylandMessageEncoder *wayland_message_encoder_new(uint32_t id, uint16_t code);

WaylandMessageEncoder *wayland_message_encoder_ref(WaylandMessageEncoder *self);

void wayland_message_encoder_unref(WaylandMessageEncoder *self);

void wayland_message_encoder_write_int(WaylandMessageEncoder *self,
                                       int32_t value);

void wayland_message_encoder_write_uint(WaylandMessageEncoder *self,
                                        uint32_t value);

void wayland_message_encoder_write_string(WaylandMessageEncoder *self,
                                          const char *value);

void wayland_message_encoder_write_object(WaylandMessageEncoder *self,
                                          uint32_t object);

void wayland_message_encoder_write_new_id(WaylandMessageEncoder *self,
                                          uint32_t new_id);

void wayland_message_encoder_write_array(WaylandMessageEncoder *self,
                                         const uint32_t *array);

void wayland_message_encoder_write_fd(WaylandMessageEncoder *self, int fd);

bool wayland_message_encoder_finish(WaylandMessageEncoder *self);

const uint8_t *wayland_message_encoder_get_data(WaylandMessageEncoder *self);

size_t wayland_message_encoder_get_length(WaylandMessageEncoder *self);
