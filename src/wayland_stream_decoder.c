#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include "ref.h"
#include "wayland_stream_decoder.h"

#define BUFFER_LENGTH 1024

struct _WaylandStreamDecoder {
  ref_t ref;
  int fd;
  WaylandStreamDecoderMessageCallback message_callback;
  void *user_data;
  void (*user_data_unref)(void *);
  uint8_t buffer[BUFFER_LENGTH];
  size_t buffer_used;
};

// Copy [data] into the buffer so it is [n_required] bytes long.
static void buffer_data(WaylandStreamDecoder *self, const uint8_t *data,
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

static void process_data(WaylandStreamDecoder *self, const uint8_t *data,
                         size_t data_length) {
  size_t data_offset = 0;
  while (true) {
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
    uint16_t length = header[1] >> 16;
    if (length < 8) {
      // FIXME: Invalid message length
      break;
    }

    // Copy over payload if using buffering.
    if (self->buffer_used > 0) {
      buffer_data(self, data, data_length, &data_offset, length);
      read_data_length = self->buffer_used;
    }

    if (read_data_length < length) {
      break;
    }

    WaylandMessageDecoder *decoder =
        wayland_message_decoder_new(read_data, length);
    self->message_callback(decoder, self->user_data);
    wayland_message_decoder_unref(decoder);

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

static void read_cb(void *user_data) {
  WaylandStreamDecoder *self = user_data;

  uint8_t data[1024];
  ssize_t data_length = read(self->fd, data, 1024);
  if (data_length == -1) {
    return;
  }

  process_data(self, data, data_length);
}

WaylandStreamDecoder *
wayland_stream_decoder_new(MainLoop *loop, int fd,
                           WaylandStreamDecoderMessageCallback message_callback,
                           void *user_data, void (*user_data_unref)(void *)) {
  WaylandStreamDecoder *self = malloc(sizeof(WaylandStreamDecoder));
  ref_init(&self->ref);
  self->fd = fd;
  self->message_callback = message_callback;
  self->user_data = user_data;
  self->user_data_unref = user_data_unref;
  self->buffer_used = 0;

  main_loop_add_fd(loop, fd, read_cb, self, NULL);

  return self;
}

WaylandStreamDecoder *wayland_stream_decoder_ref(WaylandStreamDecoder *self) {
  ref_inc(&self->ref);
  return self;
}

void wayland_stream_decoder_unref(WaylandStreamDecoder *self) {
  if (ref_dec(&self->ref)) {
    if (self->user_data_unref) {
      self->user_data_unref(self->user_data);
    }
    free(self);
  }
}
