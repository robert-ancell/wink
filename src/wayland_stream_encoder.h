#pragma once

#include <stdint.h>

#include "wayland_message_encoder.h"

typedef struct _WaylandStreamEncoder WaylandStreamEncoder;

WaylandStreamEncoder *wayland_stream_encoder_new(int fd);

WaylandStreamEncoder *wayland_stream_encoder_ref(WaylandStreamEncoder *self);

void wayland_stream_encoder_unref(WaylandStreamEncoder *self);

void wayland_stream_encoder_write(WaylandStreamEncoder *self,
                                  WaylandMessageEncoder *message);
