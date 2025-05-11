#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "wayland_message_decoder.h"

#define BUFFER_LENGTH 1024

struct _WaylandMessageDecoder {
  WaylandMessageDecoderMessageCallback message_callback;
  void *user_data;
  uint8_t buffer[BUFFER_LENGTH];
  size_t buffer_used;
};

// Copy [data] into the buffer so it is [n_required] bytes long.
static void buffer_data(WaylandMessageDecoder *self, const uint8_t *data,
                        size_t data_length, size_t *data_offset,
                        size_t n_required) {
  if (n_required > BUFFER_LENGTH) {
    n_required = BUFFER_LENGTH;
  }

  while (self->buffer_used < n_required) {
    self->buffer[self->buffer_used] = data[*data_offset];
    self->buffer_used++;
    (*data_offset)++;
  }
}

WaylandMessageDecoder *wayland_message_decoder_new(
    WaylandMessageDecoderMessageCallback message_callback, void *user_data) {
  WaylandMessageDecoder *self = malloc(sizeof(WaylandMessageDecoder));
  self->message_callback = message_callback;
  self->user_data = user_data;
  self->buffer_used = 0;

  return self;
}

WaylandMessageDecoder *
wayland_message_decoder_ref(WaylandMessageDecoder *self) {
  // FIXME
  return self;
}

void wayland_message_decoder_unref(WaylandMessageDecoder *self) {
  // FIXME
}

void wayland_message_decoder_write(WaylandMessageDecoder *self,
                                   const uint8_t *data, size_t data_length) {
  size_t total_data_length = self->buffer_used + data_length;

  size_t data_offset = 0;
  while (data_offset + 8 <= total_data_length) {
    // Copy over data for header
    if (self->buffer_used > 0 && self->buffer_used < 8) {
      buffer_data(self, data, data_length, &data_offset, 8);
    }

    // Read from buffer if full, otherwise directly from supplied data.
    const uint8_t *read_data;
    size_t read_data_length;
    if (self->buffer_used > 0) {
      read_data = self->buffer;
      read_data_length = self->buffer_used;
    } else {
      read_data = data + data_offset;
      read_data_length = data_length - data_offset;
    }

    uint32_t *header = (uint32_t *)read_data;
    uint32_t length_code = header[1];
    uint16_t length = length_code >> 16;

    if (length < 8) {
      // FIXME: Invalid
      break;
    }

    // Copy over payload if using buffering.
    if (self->buffer_used > 0) {
      buffer_data(self, data, data_length, &data_offset, length);
      read_data_length = self->buffer_used;
    }

    if (length > read_data_length) {
      break;
    }

    uint32_t id = header[0];
    uint16_t code = header[1] & 0xffff;
    const uint8_t *payload = read_data + 8;
    WaylandPayloadDecoder *decoder =
        wayland_payload_decoder_new(payload, length - 8);
    self->message_callback(id, code, decoder, self->user_data);
    wayland_payload_decoder_unref(decoder);

    // If was buffered, buffer is now empty.
    if (self->buffer_used > 0) {
      self->buffer_used = 0;
    } else {
      data_offset += length;
    }
  }

  // Copy remaining unused data into buffer.
  buffer_data(self, data, data_length, &data_offset, data_length - data_offset);
  assert(data_offset == data_length);
}
