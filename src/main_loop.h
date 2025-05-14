typedef struct _MainLoop MainLoop;

#pragma once

#include "fd.h"

typedef void (*MainLoopReadCallback)(MainLoop *self, void *user_data);

MainLoop *main_loop_new();

MainLoop *main_loop_ref(MainLoop *self);

void main_loop_unref(MainLoop *self);

void main_loop_add_fd(MainLoop *self, Fd *fd,
                      MainLoopReadCallback read_callback, void *user_data,
                      void (*user_data_unref)(void *));

void main_loop_run(MainLoop *self);
