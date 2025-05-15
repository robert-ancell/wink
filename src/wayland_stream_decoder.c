#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include "fd.h"
#include "fd_list.h"
#include "ref.h"
#include "wayland_stream_decoder.h"

#define BUFFER_LENGTH 65535
#define FD_BUFFER_LENGTH 16

struct _WaylandStreamDecoder {
  ref_t ref;
  Fd *fd;
  WaylandStreamDecoderMessageCallback message_callback;
  WaylandStreamDecoderCloseCallback close_callback;
  void *user_data;
  void (*user_data_unref)(void *);
  uint8_t buffer[BUFFER_LENGTH];
  size_t buffer_used;
  FdList *fd_list;
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
        wayland_message_decoder_new(read_data, length, self->fd_list);
    self->message_callback(self, decoder, self->user_data);
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

static void read_cb(MainLoop *loop, void *user_data) {
  WaylandStreamDecoder *self = user_data;

  struct iovec iov;
  uint8_t data[1024];
  iov.iov_base = data;
  iov.iov_len = 1024;
  uint8_t control_data[CMSG_SPACE(sizeof(int) * 1024)];
  struct msghdr msg;
  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = control_data;
  msg.msg_controllen = sizeof(control_data);
  msg.msg_flags = 0;
  ssize_t data_length = recvmsg(fd_get(self->fd), &msg, 0);
  if (data_length == -1) {
    return;
  }

  for (struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL;
       cmsg = CMSG_NXTHDR(&msg, cmsg)) {
    if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_RIGHTS) {
      size_t data_length =
          cmsg->cmsg_len - ((uint8_t *)CMSG_DATA(cmsg) - (uint8_t *)cmsg);
      size_t cmsg_fds_length = data_length / sizeof(int);
      int *cmsg_fds = (int *)CMSG_DATA(cmsg);
      for (size_t i = 0; i < cmsg_fds_length; i++) {
        Fd *fd = fd_new(cmsg_fds[i]);
        fd_list_push(self->fd_list, fd);
        fd_unref(fd);
      }
    }
  }

  if (data_length == 0) {
    self->close_callback(self, self->user_data);
    return;
  }

  process_data(self, data, data_length);
}

WaylandStreamDecoder *
wayland_stream_decoder_new(MainLoop *loop, Fd *fd,
                           WaylandStreamDecoderMessageCallback message_callback,
                           WaylandStreamDecoderCloseCallback close_callback,
                           void *user_data, void (*user_data_unref)(void *)) {
  WaylandStreamDecoder *self = malloc(sizeof(WaylandStreamDecoder));
  ref_init(&self->ref);
  self->fd = fd_ref(fd);
  self->message_callback = message_callback;
  self->close_callback = close_callback;
  self->user_data = user_data;
  self->user_data_unref = user_data_unref;
  self->buffer_used = 0;
  self->fd_list = fd_list_new();

  main_loop_add_fd(loop, fd, read_cb, self, NULL);

  return self;
}

WaylandStreamDecoder *wayland_stream_decoder_ref(WaylandStreamDecoder *self) {
  ref_inc(&self->ref);
  return self;
}

void wayland_stream_decoder_unref(WaylandStreamDecoder *self) {
  if (ref_dec(&self->ref)) {
    fd_unref(self->fd);
    if (self->user_data_unref) {
      self->user_data_unref(self->user_data);
    }
    fd_list_unref(self->fd_list);
    free(self);
  }
}
