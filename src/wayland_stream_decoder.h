#pragma once

#include <stdint.h>

#include "fd.h"
#include "main_loop.h"
#include "wayland_message_decoder.h"

typedef struct _WaylandStreamDecoder WaylandStreamDecoder;

typedef void (*WaylandStreamDecoderMessageCallback)(
    WaylandMessageDecoder *message, void *user_data);

WaylandStreamDecoder *
wayland_stream_decoder_new(MainLoop *loop, Fd *fd,
                           WaylandStreamDecoderMessageCallback message_callback,
                           void *user_data, void (*user_data_unref)(void *));

WaylandStreamDecoder *wayland_stream_decoder_ref(WaylandStreamDecoder *self);

void wayland_stream_decoder_unref(WaylandStreamDecoder *self);
