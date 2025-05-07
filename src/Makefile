BINARY = wink
SOURCES = main.c \
          main_loop.c \
          socket_server.c \
          wayland_payload_decoder.c \
          wayland_payload_encoder.c \
          wayland_server.c \
          wayland_server_client.c \
          wl_buffer_server.c \
          wl_callback_server.c \
          wl_compositor_server.c \
          wl_data_device_manager_server.c \
          wl_display_server.c \
          wl_registry_server.c \
          wl_shm_server.c \
          wl_shm_pool_server.c \
          wl_surface_server.c \
          xdg_surface_server.c \
          xdg_toplevel_server.c \
          xdg_wm_base_server.c
OBJECTS = $(SOURCES:.c=.o)
CFLAGS = -g -Wall

%.o: %.c
	gcc $(CFLAGS) -c $<

$(BINARY): $(OBJECTS)
	gcc $^ -o $@

clean:
	rm -f $(BINARY) $(OBJECTS)

