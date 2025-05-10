#include <stdlib.h>

#include "main_loop.h"
#include "wayland_client.h"
#include "wayland_server.h"

int main(int argc, char **argv) {
  MainLoop *loop = main_loop_new();

  WaylandClient *client = wayland_client_new(loop);
  wayland_client_connect(client, NULL);

  WaylandServer *server = wayland_server_new(loop);
  if (!wayland_server_run(server, "wayland-99")) {
    return 1;
  }

  main_loop_run(loop);

  return 0;
}
