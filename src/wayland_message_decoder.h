#pragma once

#include <stdint.h>

#include "wayland_payload_decoder.h"

typedef struct _WaylandMessageDecoder WaylandMessageDecoder;

typedef void (*WaylandMessageDecoderMessageCallback)(
    uint32_t id, uint16_t code, WaylandPayloadDecoder *payload,
    void *user_data);

WaylandMessageDecoder *wayland_message_decoder_new(
    WaylandMessageDecoderMessageCallback message_callback, void *user_data);

WaylandMessageDecoder *wayland_message_decoder_ref(WaylandMessageDecoder *self);

void wayland_message_decoder_unref(WaylandMessageDecoder *self);

void wayland_message_decoder_write(WaylandMessageDecoder *self,
                                   const uint8_t *data, size_t data_length);
