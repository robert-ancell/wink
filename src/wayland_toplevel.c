#include <stdlib.h>

#include "wayland_toplevel.h"

#include "wl_surface_client.h"
#include "xdg_surface_client.h"
#include "xdg_toplevel_client.h"

struct _WaylandToplevel {
  WlSurfaceClient *surface;
  XdgSurfaceClient *xdg_surface;
  XdgToplevelClient *xdg_toplevel;
};

static XdgToplevelClientEventCallbacks xdg_toplevel_callbacks = {};

static XdgSurfaceClientEventCallbacks xdg_surface_callbacks = {};

static WlSurfaceClientEventCallbacks surface_callbacks = {};

WaylandToplevel *wayland_toplevel_new(WaylandClient *client) {
  WaylandToplevel *self = malloc(sizeof(WaylandToplevel));

  self->surface = wl_surface_client_new(client, &surface_callbacks, self);
  wl_compositor_client_create_surface(wayland_client_get_compositor(client),
                                      wl_surface_client_get_id(self->surface));

  self->xdg_surface =
      xdg_surface_client_new(client, &xdg_surface_callbacks, self);
  xdg_wm_base_client_get_xdg_surface(
      wayland_client_get_wm_base(client),
      xdg_surface_client_get_id(self->xdg_surface),
      wl_surface_client_get_id(self->surface));

  self->xdg_toplevel =
      xdg_toplevel_client_new(client, &xdg_toplevel_callbacks, self);
  xdg_surface_client_get_toplevel(
      self->xdg_surface, xdg_toplevel_client_get_id(self->xdg_toplevel));

  return self;
}

WaylandToplevel *wayland_toplevel_ref(WaylandToplevel *self) {
  // FIXME
  return self;
}

void wayland_toplevel_unref(WaylandToplevel *self) {
  // FIXME
}
