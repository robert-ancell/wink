typedef struct _Fd Fd;

#pragma once

Fd *fd_new(int fd);

Fd *fd_ref(Fd *self);

void fd_unref(Fd *self);

int fd_get(Fd *self);

int fd_take(Fd *self);
