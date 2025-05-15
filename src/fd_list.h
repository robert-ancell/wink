typedef struct _FdList FdList;

#pragma once

#include "fd.h"

FdList *fd_list_new();

FdList *fd_list_ref(FdList *self);

void fd_list_unref(FdList *self);

void fd_list_push(FdList *self, Fd *fd);

Fd *fd_list_pop(FdList *self);
