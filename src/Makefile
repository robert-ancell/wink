BINARY = wink
WAYLAND_SERVER_INTERFACES = wl_buffer \
                            wl_callback \
                            wl_compositor \
                            wl_data_device_manager \
                            wl_display \
                            wl_registry \
                            wl_shm_pool \
                            wl_shm \
                            wl_surface
XDG_SERVER_INTERFACES = xdg_surface \
                        xdg_toplevel \
                        xdg_wm_base
WAYLAND_CLIENT_INTERFACES = wl_display \
                            wl_registry \
                            wl_shm
XDG_CLIENT_INTERFACES = xdg_surface \
                        xdg_toplevel
WAYLAND_SERVER_SOURCES = $(WAYLAND_SERVER_INTERFACES:%=%_server.c) $(WAYLAND_SERVER_INTERFACES:%=%_server.h)
XDG_SERVER_SOURCES = $(XDG_SERVER_INTERFACES:%=%_server.c) $(XDG_SERVER_INTERFACES:%=%_server.h)
WAYLAND_CLIENT_SOURCES = $(WAYLAND_CLIENT_INTERFACES:%=%_client.c) $(WAYLAND_CLIENT_INTERFACES:%=%_client.h)
XDG_CLIENT_SOURCES = $(XDG_CLIENT_INTERFACES:%=%_client.c) $(XDG_CLIENT_INTERFACES:%=%_client.h)
SOURCES = main.c \
          main_loop.c \
          socket_client.c \
          socket_server.c \
          wayland_client.c \
          wayland_payload_decoder.c \
          wayland_payload_encoder.c \
          wayland_server.c \
          wayland_server_client.c
GENERATED_SOURCES = $(WAYLAND_SERVER_SOURCES) \
                    $(XDG_SERVER_SOURCES) \
                    $(WAYLAND_CLIENT_SOURCES) \
                    $(XDG_CLIENT_SOURCES)
OBJECTS = $(SOURCES:.c=.o)
GENERATED_OBJECTS = $(GENERATED_SOURCES:.c=.o)
CFLAGS = -g -Wall

$(BINARY): $(GENERATED_OBJECTS) $(OBJECTS)
	gcc $^ -o $@

$(WAYLAND_SERVER_SOURCES): generate-wayland.py wayland.xml
	./generate-wayland.py wayland.xml server $(WAYLAND_SERVER_INTERFACES)

$(XDG_SERVER_SOURCES): generate-wayland.py xdg-shell.xml
	./generate-wayland.py xdg-shell.xml server $(XDG_SERVER_INTERFACES)

$(WAYLAND_CLIENT_SOURCES): generate-wayland.py wayland.xml
	./generate-wayland.py wayland.xml client $(WAYLAND_CLIENT_INTERFACES)

$(XDG_CLIENT_SOURCES): generate-wayland.py xdg-shell.xml
	./generate-wayland.py xdg-shell.xml client $(XDG_CLIENT_INTERFACES)

%.o: %.c
	gcc $(CFLAGS) -c $<

clean:
	rm -f $(BINARY) $(GENERATED_OBJECTS) $(OBJECTS) $(GENERATED_SOURCES)
