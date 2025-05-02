#pragma once

typedef struct _MainLoop MainLoop;

typedef void (*MainLoopReadCallback)(void *user_data);

MainLoop *main_loop_new();

MainLoop *main_loop_ref(MainLoop *self);

void main_loop_unref(MainLoop *self);

void main_loop_add_fd(MainLoop *self, int fd,
                      MainLoopReadCallback read_callback, void *user_data);

void main_loop_run(MainLoop *self);
