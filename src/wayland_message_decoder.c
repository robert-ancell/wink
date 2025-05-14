#include <stdbool.h>
#include <stdlib.h>

#include "wayland_message_decoder.h"

#include "ref.h"

struct _WaylandMessageDecoder {
  ref_t ref;
  const uint8_t *data;
  size_t data_length;
  size_t offset;
  uint32_t id;
  uint16_t code;
  bool error;
};

WaylandMessageDecoder *wayland_message_decoder_new(const uint8_t *data,
                                                   size_t data_length) {
  WaylandMessageDecoder *self = malloc(sizeof(WaylandMessageDecoder));
  ref_init(&self->ref);
  self->data = data;
  self->data_length = data_length;
  self->offset = 0;
  self->error = false;

  self->id = wayland_message_decoder_read_uint(self);
  uint32_t length_and_code = wayland_message_decoder_read_uint(self);
  self->code = length_and_code & 0xffff;
  uint16_t length = length_and_code >> 16;
  if (length != data_length) {
    self->error = true;
  }

  return self;
}

WaylandMessageDecoder *
wayland_message_decoder_ref(WaylandMessageDecoder *self) {
  ref_inc(&self->ref);
  return self;
}

void wayland_message_decoder_unref(WaylandMessageDecoder *self) {
  if (ref_dec(&self->ref)) {
    free(self);
  }
}

uint32_t wayland_message_decoder_get_id(WaylandMessageDecoder *self) {
  return self->id;
}

uint16_t wayland_message_decoder_get_code(WaylandMessageDecoder *self) {
  return self->code;
}

int32_t wayland_message_decoder_read_int(WaylandMessageDecoder *self) {
  if (self->offset + 4 > self->data_length) {
    self->error = true;
    return 0;
  }
  int32_t value = *(int32_t *)(self->data + self->offset);
  self->offset += 4;
  return value;
}

uint32_t wayland_message_decoder_read_uint(WaylandMessageDecoder *self) {
  if (self->offset + 4 > self->data_length) {
    self->error = true;
    return 0;
  }
  uint32_t value = *(uint32_t *)(self->data + self->offset);
  self->offset += 4;
  return value;
}

const char *wayland_message_decoder_read_string(WaylandMessageDecoder *self) {
  uint32_t length = wayland_message_decoder_read_uint(self);
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

uint32_t wayland_message_decoder_read_object(WaylandMessageDecoder *self) {
  return wayland_message_decoder_read_uint(self);
}

uint32_t wayland_message_decoder_read_new_id(WaylandMessageDecoder *self) {
  return wayland_message_decoder_read_uint(self);
}

uint32_t *wayland_message_decoder_read_array(WaylandMessageDecoder *self) {
  // FIXME: Implement array support
  return NULL;
}

int wayland_message_decoder_read_fd(WaylandMessageDecoder *self) {
  // FIXME: Implement fd support
  return -1;
}

bool wayland_message_decoder_finish(WaylandMessageDecoder *self) {
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
