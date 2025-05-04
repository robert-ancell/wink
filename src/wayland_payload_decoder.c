#include <stdbool.h>
#include <stdlib.h>

#include "wayland_payload_decoder.h"

struct _WaylandPayloadDecoder {
  const uint8_t *data;
  size_t data_length;
  size_t offset;
  bool error;
};

WaylandPayloadDecoder *wayland_payload_decoder_new(const uint8_t *data,
                                                   size_t data_length) {
  WaylandPayloadDecoder *self = malloc(sizeof(WaylandPayloadDecoder));
  self->data = data;
  self->data_length = data_length;
  self->offset = 0;
  self->error = false;

  return self;
}

WaylandPayloadDecoder *
wayland_payload_decoder_ref(WaylandPayloadDecoder *self) {
  // FIXME
  return self;
}

void wayland_payload_decoder_unref(WaylandPayloadDecoder *self) {
  // FIXME
}

int32_t wayland_payload_decoder_read_int(WaylandPayloadDecoder *self) {
  if (self->offset + 4 > self->data_length) {
    self->error = true;
    return 0;
  }
  int32_t value = *(int32_t *)(self->data + self->offset);
  self->offset += 4;
  return value;
}

uint32_t wayland_payload_decoder_read_uint(WaylandPayloadDecoder *self) {
  if (self->offset + 4 > self->data_length) {
    self->error = true;
    return 0;
  }
  uint32_t value = *(uint32_t *)(self->data + self->offset);
  self->offset += 4;
  return value;
}

const char *wayland_payload_decoder_read_string(WaylandPayloadDecoder *self) {
  uint32_t length = wayland_payload_decoder_read_uint(self);
  if (self->error || self->offset + length + 1 > self->data_length) {
    return "";
  }
  const char *value = (const char *)(self->data + self->offset);
  // Check nul terminated
  if (value[length - 1] != '\0') {
    self->error = true;
    return "";
  }
  self->offset += length;
  // Skip alignment
  while (self->offset % 4 != 0) {
    self->offset++;
  }
  return value;
}

uint32_t wayland_payload_decoder_read_object(WaylandPayloadDecoder *self) {
  return wayland_payload_decoder_read_uint(self);
}

uint32_t wayland_payload_decoder_read_new_id(WaylandPayloadDecoder *self) {
  return wayland_payload_decoder_read_uint(self);
}

uint32_t *wayland_payload_decoder_read_array(WaylandPayloadDecoder *self) {
  // FIXME
  return NULL;
}

int wayland_payload_decoder_read_fd(WaylandPayloadDecoder *self) {
  // FIXME
  return -1;
}

bool wayland_payload_decoder_finish(WaylandPayloadDecoder *self) {
  // Error occurred during decoding.
  if (self->error) {
    return false;
  }

  // Unused data.
  if (self->offset != self->data_length) {
    return false;
  }

  return true;
}
