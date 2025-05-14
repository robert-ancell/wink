typedef struct _WinkServer WinkServer;

#pragma once

#include <stdbool.h>

#include "main_loop.h"

WinkServer *wink_server_new(MainLoop *loop);

WinkServer *wink_server_ref(WinkServer *self);

void wink_server_unref(WinkServer *self);

bool wink_server_run(WinkServer *self);
