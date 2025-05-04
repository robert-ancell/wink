#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "wayland_payload_encoder.h"

struct _WaylandPayloadEncoder {
  // FIXME: Make dynamic
  uint8_t data[1024];
  size_t data_length;
  size_t offset;
  bool error;
};

WaylandPayloadEncoder *wayland_payload_encoder_new() {
  WaylandPayloadEncoder *self = malloc(sizeof(WaylandPayloadEncoder));
  self->data_length = 1024;
  self->offset = 0;
  self->error = false;

  return self;
}

WaylandPayloadEncoder *
wayland_payload_encoder_ref(WaylandPayloadEncoder *self) {
  // FIXME
  return self;
}

void wayland_payload_encoder_unref(WaylandPayloadEncoder *self) {
  // FIXME
}

void wayland_payload_encoder_write_int(WaylandPayloadEncoder *self,
                                       int32_t value) {
  if (self->offset + 4 > self->data_length) {
    self->error = true;
    return;
  }

  *(int32_t *)(self->data + self->offset) = value;
  self->offset += 4;
}

void wayland_payload_encoder_write_uint(WaylandPayloadEncoder *self,
                                        uint32_t value) {
  if (self->offset + 4 > self->data_length) {
    self->error = true;
    return;
  }

  *(uint32_t *)(self->data + self->offset) = value;
  self->offset += 4;
}

void wayland_payload_encoder_write_string(WaylandPayloadEncoder *self,
                                          const char *value) {
  size_t value_length = strlen(value);
  wayland_payload_encoder_write_uint(self, value_length + 1);
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

void wayland_payload_encoder_write_object(WaylandPayloadEncoder *self,
                                          uint32_t object) {
  wayland_payload_encoder_write_uint(self, object);
}

void wayland_payload_encoder_write_new_id(WaylandPayloadEncoder *self,
                                          uint32_t new_id) {
  wayland_payload_encoder_write_uint(self, new_id);
}

void wayland_payload_encoder_write_array(WaylandPayloadEncoder *self,
                                         const uint32_t *array) {
  // FIXME
}

void wayland_payload_encoder_write_fd(WaylandPayloadEncoder *self, int fd) {
  // FIXME
}

bool wayland_payload_encoder_finish(WaylandPayloadEncoder *self) {
  return !self->error;
}

const uint8_t *wayland_payload_encoder_get_data(WaylandPayloadEncoder *self) {
  return self->data;
}

size_t wayland_payload_encoder_get_length(WaylandPayloadEncoder *self) {
  return self->offset;
}
