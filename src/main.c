#include "wink_server.h"

int main(int argc, char **argv) {
  MainLoop *loop = main_loop_new();

  WinkServer *server = wink_server_new(loop);
  if (!wink_server_run(server)) {
    return 1;
  }

  main_loop_run(loop);

  wink_server_unref(server);
  main_loop_unref(loop);

  return 0;
}
