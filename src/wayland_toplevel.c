#include <stdlib.h>

#include "wayland_toplevel.h"

#include "ref.h"
#include "wl_surface_client.h"
#include "xdg_surface_client.h"
#include "xdg_toplevel_client.h"

struct _WaylandToplevel {
  ref_t ref;
  WlSurfaceClient *surface;
  XdgSurfaceClient *xdg_surface;
  XdgToplevelClient *xdg_toplevel;
};

static XdgToplevelClientEventCallbacks xdg_toplevel_callbacks = {};

static XdgSurfaceClientEventCallbacks xdg_surface_callbacks = {};

static WlSurfaceClientEventCallbacks surface_callbacks = {};

WaylandToplevel *wayland_toplevel_new(WaylandClient *client) {
  WaylandToplevel *self = malloc(sizeof(WaylandToplevel));
  ref_init(&self->ref);
  self->surface = wl_surface_client_new(client, &surface_callbacks, self, NULL);
  wl_compositor_client_create_surface(wayland_client_get_compositor(client),
                                      wl_surface_client_get_id(self->surface));

  self->xdg_surface =
      xdg_surface_client_new(client, &xdg_surface_callbacks, self, NULL);
  xdg_wm_base_client_get_xdg_surface(
      wayland_client_get_wm_base(client),
      xdg_surface_client_get_id(self->xdg_surface),
      wl_surface_client_get_id(self->surface));

  self->xdg_toplevel =
      xdg_toplevel_client_new(client, &xdg_toplevel_callbacks, self, NULL);
  xdg_surface_client_get_toplevel(
      self->xdg_surface, xdg_toplevel_client_get_id(self->xdg_toplevel));

  return self;
}

WaylandToplevel *wayland_toplevel_ref(WaylandToplevel *self) {
  ref_inc(&self->ref);
  return self;
}

void wayland_toplevel_unref(WaylandToplevel *self) {
  if (ref_dec(&self->ref)) {
    wl_surface_client_unref(self->surface);
    xdg_surface_client_unref(self->xdg_surface);
    xdg_toplevel_client_unref(self->xdg_toplevel);
    free(self);
  }
}

void wayland_toplevel_destroy(WaylandToplevel *self) {
  xdg_toplevel_client_destroy(self->xdg_toplevel);
}

void wayland_toplevel_set_title(WaylandToplevel *self, const char *title) {
  xdg_toplevel_client_set_title(self->xdg_toplevel, title);
}

void wayland_toplevel_set_app_id(WaylandToplevel *self, const char *app_id) {
  xdg_toplevel_client_set_app_id(self->xdg_toplevel, app_id);
}
