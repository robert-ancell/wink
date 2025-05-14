#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include "wayland_stream_encoder.h"

#define BUFFER_LENGTH 1024

struct _WaylandStreamEncoder {
  int fd;
};

WaylandStreamEncoder *wayland_stream_encoder_new(int fd) {
  WaylandStreamEncoder *self = malloc(sizeof(WaylandStreamEncoder));
  self->fd = fd;

  return self;
}

WaylandStreamEncoder *wayland_stream_encoder_ref(WaylandStreamEncoder *self) {
  // FIXME
  return self;
}

void wayland_stream_encoder_unref(WaylandStreamEncoder *self) {
  // FIXME
}

void wayland_stream_encoder_write(WaylandStreamEncoder *self,
                                  WaylandMessageEncoder *message) {
  size_t message_length = wayland_message_encoder_get_length(message);
  assert(write(self->fd, wayland_message_encoder_get_data(message),
               message_length) == message_length);
}
