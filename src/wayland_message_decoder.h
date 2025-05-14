#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "fd.h"
#include "main_loop.h"

typedef struct _WaylandMessageDecoder WaylandMessageDecoder;

WaylandMessageDecoder *wayland_message_decoder_new(const uint8_t *data,
                                                   size_t data_length);

WaylandMessageDecoder *wayland_message_decoder_ref(WaylandMessageDecoder *self);

void wayland_message_decoder_unref(WaylandMessageDecoder *self);

uint32_t wayland_message_decoder_get_id(WaylandMessageDecoder *self);

uint16_t wayland_message_decoder_get_code(WaylandMessageDecoder *self);

int32_t wayland_message_decoder_read_int(WaylandMessageDecoder *self);

uint32_t wayland_message_decoder_read_uint(WaylandMessageDecoder *self);

const char *wayland_message_decoder_read_string(WaylandMessageDecoder *self);

uint32_t wayland_message_decoder_read_object(WaylandMessageDecoder *self);

uint32_t wayland_message_decoder_read_new_id(WaylandMessageDecoder *self);

uint32_t *wayland_message_decoder_read_array(WaylandMessageDecoder *self);

Fd *wayland_message_decoder_read_fd(WaylandMessageDecoder *self);

bool wayland_message_decoder_finish(WaylandMessageDecoder *self);
