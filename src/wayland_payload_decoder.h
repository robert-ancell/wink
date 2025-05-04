#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "main_loop.h"

typedef struct _WaylandPayloadDecoder WaylandPayloadDecoder;

WaylandPayloadDecoder *wayland_payload_decoder_new(const uint8_t *data,
                                                   size_t data_length);

WaylandPayloadDecoder *wayland_payload_decoder_ref(WaylandPayloadDecoder *self);

void wayland_payload_decoder_unref(WaylandPayloadDecoder *self);

int32_t wayland_payload_decoder_read_int(WaylandPayloadDecoder *self);

uint32_t wayland_payload_decoder_read_uint(WaylandPayloadDecoder *self);

const char *wayland_payload_decoder_read_string(WaylandPayloadDecoder *self);

uint32_t wayland_payload_decoder_read_object(WaylandPayloadDecoder *self);

uint32_t wayland_payload_decoder_read_new_id(WaylandPayloadDecoder *self);

uint32_t *wayland_payload_decoder_read_array(WaylandPayloadDecoder *self);

int wayland_payload_decoder_read_fd(WaylandPayloadDecoder *self);

bool wayland_payload_decoder_finish(WaylandPayloadDecoder *self);
