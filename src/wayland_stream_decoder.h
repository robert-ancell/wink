typedef struct _WaylandStreamDecoder WaylandStreamDecoder;

#pragma once

#include <stdint.h>

#include "fd.h"
#include "main_loop.h"
#include "wayland_message_decoder.h"

typedef void (*WaylandStreamDecoderMessageCallback)(
    WaylandStreamDecoder *self, WaylandMessageDecoder *message,
    void *user_data);

typedef void (*WaylandStreamDecoderCloseCallback)(WaylandStreamDecoder *self,
                                                  void *user_data);

WaylandStreamDecoder *
wayland_stream_decoder_new(MainLoop *loop, Fd *fd,
                           WaylandStreamDecoderMessageCallback message_callback,
                           WaylandStreamDecoderCloseCallback close_callback,
                           void *user_data, void (*user_data_unref)(void *));

WaylandStreamDecoder *wayland_stream_decoder_ref(WaylandStreamDecoder *self);

void wayland_stream_decoder_unref(WaylandStreamDecoder *self);
