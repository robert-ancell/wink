#pragma once

#include <stdint.h>

#include "wayland_message_decoder.h"

typedef struct _WaylandStreamDecoder WaylandStreamDecoder;

typedef void (*WaylandStreamDecoderMessageCallback)(
    WaylandMessageDecoder *message, void *user_data);

WaylandStreamDecoder *
wayland_stream_decoder_new(WaylandStreamDecoderMessageCallback message_callback,
                           void *user_data);

WaylandStreamDecoder *wayland_stream_decoder_ref(WaylandStreamDecoder *self);

void wayland_stream_decoder_unref(WaylandStreamDecoder *self);

void wayland_stream_decoder_write(WaylandStreamDecoder *self,
                                  const uint8_t *data, size_t data_length);
