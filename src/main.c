#include <stdio.h>

#include "main_loop.h"
#include "socket_server.h"

static void connect_cb(int fd, void *user_data) { printf("%d\n", fd); }

int main(int argc, char **argv) {
  MainLoop *loop = main_loop_new();

  SocketServer *server = socket_server_new(loop, connect_cb, NULL);
  socket_server_run(server);

  main_loop_run(loop);
}
