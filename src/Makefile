BINARY = wink
WAYLAND_INTERFACES = wl_buffer \
                     wl_callback \
                     wl_compositor \
                     wl_data_device_manager \
                     wl_display \
                     wl_registry \
                     wl_shm_pool \
                     wl_shm \
                     wl_surface
XDG_INTERFACES = xdg_surface \
                 xdg_toplevel \
                 xdg_wm_base
WAYLAND_SOURCES = $(WAYLAND_INTERFACES:%=%_server.c) $(WAYLAND_INTERFACES:%=%_server.h)
XDG_SOURCES = $(XDG_INTERFACES:%=%_server.c) $(XDG_INTERFACES:%=%_server.h)
SOURCES = main.c \
          main_loop.c \
          socket_server.c \
          wayland_payload_decoder.c \
          wayland_payload_encoder.c \
          wayland_server.c \
          wayland_server_client.c
GENERATED_SOURCES = $(WAYLAND_SOURCES) \
                    $(XDG_SOURCES)
OBJECTS = $(SOURCES:.c=.o)
GENERATED_OBJECTS = $(GENERATED_SOURCES:.c=.o)
CFLAGS = -g -Wall

$(BINARY): $(GENERATED_OBJECTS) $(OBJECTS)
	gcc $^ -o $@

$(WAYLAND_SOURCES): wayland.xml
	./generate-wayland.py wayland.xml server $(WAYLAND_INTERFACES)

$(XDG_SOURCES): xdg-shell.xml
	./generate-wayland.py xdg-shell.xml server $(XDG_INTERFACES)

%.o: %.c
	gcc $(CFLAGS) -c $<

clean:
	rm -f $(BINARY) $(GENERATED_OBJECTS) $(OBJECTS) $(GENERATED_SOURCES)
