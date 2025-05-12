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
XDG_SHELL_SERVER_INTERFACES = xdg_surface \
                              xdg_toplevel \
                              xdg_wm_base
LINUX_DMABUF_SERVER_INTERFACES = zwp_linux_buffer_params_v1 \
                                 zwp_linux_dmabuf_feedback_v1 \
                                 zwp_linux_dmabuf_v1
WAYLAND_CLIENT_INTERFACES = wl_buffer \
                            wl_callback \
                            wl_compositor \
                            wl_display \
                            wl_registry \
                            wl_shm_pool \
                            wl_shm \
                            wl_surface
XDG_SHELL_CLIENT_INTERFACES = xdg_surface \
                              xdg_toplevel \
                              xdg_wm_base
LINUX_DMABUF_CLIENT_INTERFACES = zwp_linux_buffer_params_v1 \
                                 zwp_linux_dmabuf_feedback_v1 \
                                 zwp_linux_dmabuf_v1
WAYLAND_SERVER_SOURCES = $(WAYLAND_SERVER_INTERFACES:%=%_server.c)
XDG_SHELL_SERVER_SOURCES = $(XDG_SHELL_SERVER_INTERFACES:%=%_server.c)
LINUX_DMABUF_SERVER_SOURCES = $(LINUX_DMABUF_SERVER_INTERFACES:%=%_server.c)
WAYLAND_CLIENT_SOURCES = $(WAYLAND_CLIENT_INTERFACES:%=%_client.c)
XDG_SHELL_CLIENT_SOURCES = $(XDG_SHELL_CLIENT_INTERFACES:%=%_client.c)
LINUX_DMABUF_CLIENT_SOURCES = $(LINUX_DMABUF_CLIENT_INTERFACES:%=%_client.c)
SOURCES = main.c \
          main_loop.c \
          socket_client.c \
          socket_server.c \
          wayland_client.c \
          wayland_message_decoder.c \
          wayland_payload_decoder.c \
          wayland_payload_encoder.c \
          wayland_server.c \
          wayland_server_client.c \
          wayland_toplevel.c
GENERATED_SOURCES = $(WAYLAND_SERVER_SOURCES) \
                    $(XDG_SHELL_SERVER_SOURCES) \
                    $(LINUX_DMABUF_SERVER_SOURCES) \
                    $(WAYLAND_CLIENT_SOURCES) \
                    $(XDG_SHELL_CLIENT_SOURCES) \
                    $(LINUX_DMABUF_CLIENT_SOURCES)
OBJECTS = $(SOURCES:.c=.o)
GENERATED_OBJECTS = $(GENERATED_SOURCES:.c=.o)
CFLAGS = -g -Wall

$(BINARY): $(GENERATED_OBJECTS) $(OBJECTS)
	gcc $^ -o $@

$(WAYLAND_SERVER_SOURCES) $(WAYLAND_SERVER_SOURCES:.c=.h): generate-wayland.py wayland.xml
	./generate-wayland.py wayland.xml server $(WAYLAND_SERVER_INTERFACES)

$(XDG_SHELL_SERVER_SOURCES) $(XDG_SHELL_SERVER_SOURCES:.c=.h): generate-wayland.py xdg-shell.xml
	./generate-wayland.py xdg-shell.xml server $(XDG_SHELL_SERVER_INTERFACES)

$(LINUX_DMABUF_SERVER_SOURCES) $(LINUX_DMABUF_SERVER_SOURCES:.c=.h): generate-wayland.py linux-dmabuf-v1.xml
	./generate-wayland.py linux-dmabuf-v1.xml server $(LINUX_DMABUF_SERVER_INTERFACES)

$(WAYLAND_CLIENT_SOURCES) $(WAYLAND_CLIENT_SOURCES:.c=.h): generate-wayland.py wayland.xml
	./generate-wayland.py wayland.xml client $(WAYLAND_CLIENT_INTERFACES)

$(XDG_SHELL_CLIENT_SOURCES) $(XDG_SHELL_CLIENT_SOURCES:.c=.h): generate-wayland.py xdg-shell.xml
	./generate-wayland.py xdg-shell.xml client $(XDG_SHELL_CLIENT_INTERFACES)

$(LINUX_DMABUF_CLIENT_SOURCES) $(LINUX_DMABUF_CLIENT_SOURCES:.c=.h): generate-wayland.py linux-dmabuf-v1.xml
	./generate-wayland.py linux-dmabuf-v1.xml client $(LINUX_DMABUF_CLIENT_INTERFACES)

%.o: %.c
	gcc $(CFLAGS) -c $<

clean:
	rm -f $(BINARY) $(GENERATED_OBJECTS) $(OBJECTS) $(GENERATED_SOURCES)
