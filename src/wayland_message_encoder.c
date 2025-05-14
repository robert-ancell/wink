#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "wayland_message_encoder.h"

struct _WaylandMessageEncoder {
  // FIXME: Make dynamic
  uint8_t data[1024];
  size_t data_length;
  size_t offset;
  int *fds;
  size_t fds_length;
  bool error;
};

WaylandMessageEncoder *wayland_message_encoder_new(uint32_t id, uint16_t code) {
  WaylandMessageEncoder *self = malloc(sizeof(WaylandMessageEncoder));
  self->data_length = 1024;
  self->offset = 0;
  self->fds = NULL;
  self->fds_length = 0;
  self->error = false;

  wayland_message_encoder_write_uint(self, id);
  wayland_message_encoder_write_uint(self, code);

  return self;
}

WaylandMessageEncoder *
wayland_message_encoder_ref(WaylandMessageEncoder *self) {
  // FIXME
  return self;
}

void wayland_message_encoder_unref(WaylandMessageEncoder *self) {
  // FIXME
}

void wayland_message_encoder_write_int(WaylandMessageEncoder *self,
                                       int32_t value) {
  if (self->offset + 4 > self->data_length) {
    self->error = true;
    return;
  }

  *(int32_t *)(self->data + self->offset) = value;
  self->offset += 4;
}

void wayland_message_encoder_write_uint(WaylandMessageEncoder *self,
                                        uint32_t value) {
  if (self->offset + 4 > self->data_length) {
    self->error = true;
    return;
  }

  *(uint32_t *)(self->data + self->offset) = value;
  self->offset += 4;
}

void wayland_message_encoder_write_string(WaylandMessageEncoder *self,
                                          const char *value) {
  size_t value_length = strlen(value);
  wayland_message_encoder_write_uint(self, value_length + 1);
  if (self->offset + value_length + 1 > self->data_length) {
    self->error = true;
    return;
  }
  for (size_t i = 0; i <= value_length; i++) {
    self->data[self->offset++] = value[i];
  }
  while (self->offset % 4 != 0) {
    if (self->offset >= self->data_length) {
      self->error = true;
      return;
    }
    self->data[self->offset++] = 0x00;
  }
}

void wayland_message_encoder_write_object(WaylandMessageEncoder *self,
                                          uint32_t object) {
  wayland_message_encoder_write_uint(self, object);
}

void wayland_message_encoder_write_new_id(WaylandMessageEncoder *self,
                                          uint32_t new_id) {
  wayland_message_encoder_write_uint(self, new_id);
}

void wayland_message_encoder_write_array(WaylandMessageEncoder *self,
                                         const uint32_t *array) {
  // FIXME
}

void wayland_message_encoder_write_fd(WaylandMessageEncoder *self, int fd) {
  self->fds_length++;
  self->fds = realloc(self->fds, sizeof(int) * self->fds_length);
  self->fds[self->fds_length - 1] = fd;
}

bool wayland_message_encoder_finish(WaylandMessageEncoder *self) {
  // Message too long
  if (self->offset > 0xffff) {
    return false;
  }

  // Write length field.
  uint32_t *header = (uint32_t *)self->data;
  header[1] |= self->offset << 16;

  return !self->error;
}

const uint8_t *wayland_message_encoder_get_data(WaylandMessageEncoder *self) {
  return self->data;
}

size_t wayland_message_encoder_get_length(WaylandMessageEncoder *self) {
  return self->offset;
}

const int *wayland_message_encoder_get_fds(WaylandMessageEncoder *self) {
  return self->fds;
}

size_t wayland_message_encoder_get_fds_length(WaylandMessageEncoder *self) {
  return self->fds_length;
}
