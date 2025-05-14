typedef struct _WaylandStreamEncoder WaylandStreamEncoder;

#pragma once

#include <stdint.h>

#include "fd.h"
#include "wayland_message_encoder.h"

WaylandStreamEncoder *wayland_stream_encoder_new(Fd *fd);

WaylandStreamEncoder *wayland_stream_encoder_ref(WaylandStreamEncoder *self);

void wayland_stream_encoder_unref(WaylandStreamEncoder *self);

void wayland_stream_encoder_write(WaylandStreamEncoder *self,
                                  WaylandMessageEncoder *message);
