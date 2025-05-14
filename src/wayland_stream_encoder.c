#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

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
  const uint8_t *data = wayland_message_encoder_get_data(message);
  size_t data_length = wayland_message_encoder_get_length(message);
  const int *fds = wayland_message_encoder_get_fds(message);
  size_t fds_length = wayland_message_encoder_get_fds_length(message);

  struct iovec iov;
  iov.iov_base = (void *)data;
  iov.iov_len = data_length;
  uint8_t control_data[CMSG_SPACE(sizeof(int) * fds_length)];
  memset(control_data, 0, sizeof(control_data));
  struct msghdr msg;
  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = control_data;
  msg.msg_controllen = sizeof(control_data);
  msg.msg_flags = 0;
  struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type = SCM_RIGHTS;
  cmsg->cmsg_len = CMSG_LEN(sizeof(int) * fds_length);
  int *cmsg_fds = (int *)CMSG_DATA(cmsg);
  for (size_t i = 0; i < fds_length; i++) {
    cmsg_fds[i] = fds[i];
  }
  assert(sendmsg(self->fd, &msg, 0) == data_length);
}
