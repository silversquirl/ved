#include "ui.h"

static int ui_colour(struct ui *ui, XftColor *c, const char *name) {
  Visual *visual = XftDrawVisual(ui->text.draw);
  Colormap cmap = XftDrawColormap(ui->text.draw);
  return !XftColorAllocName(ui->dpy, visual, cmap, name, c);
}

static void ui_render(struct ui *ui) {
  XClearWindow(ui->dpy, ui->w);

  size_t n;
  char *text = rope_flatten(ui->ved->buffer, &n);
  pango_layout_set_text(ui->text.layout, text, n);

  pango_xft_render_layout(ui->text.draw, &ui->text.fg, ui->text.layout, 5000, 0);

  XFlush(ui->dpy);
}

int ui_init(struct ui *ui, struct editor *ved) {
  ui->ved = ved;

  // X11
  ui->dpy = XOpenDisplay(NULL);
  if (!ui->dpy) return 1;
  int scr = DefaultScreen(ui->dpy);
  int black = BlackPixel(ui->dpy, scr);
  ui->w = XCreateSimpleWindow(ui->dpy, DefaultRootWindow(ui->dpy), 0, 0, 600, 400, 0, black, black);
  XSelectInput(ui->dpy, ui->w, ExposureMask | StructureNotifyMask);

  // Pango
  PangoFontMap *fm  = pango_xft_get_font_map(ui->dpy, scr);
  PangoContext *ctx = pango_font_map_create_context(fm);
  ui->text.layout  = pango_layout_new(ctx);

  PangoFontDescription *fdesc = pango_font_description_from_string("Helvetica 11");
  pango_layout_set_font_description(ui->text.layout, fdesc);
  pango_font_description_free(fdesc);

  // Xft
  Visual *v = DefaultVisual(ui->dpy, scr);
  Colormap cmap = DefaultColormap(ui->dpy, scr);
  ui->text.draw = XftDrawCreate(ui->dpy, ui->w, v, cmap);
  ui_colour(ui, &ui->text.fg, "white");

  return 0;
}

void ui_mainloop(struct ui *ui) {
  XMapWindow(ui->dpy, ui->w);
  XEvent e;
  for (;;) {
    XNextEvent(ui->dpy, &e);
    switch (e.type) {
    case Expose:
      ui_render(ui);
      break;

    case ConfigureNotify:
      if (e.xconfigure.width != ui->dim.w || e.xconfigure.height != ui->dim.h) {
        ui->dim.w = e.xconfigure.width;
        ui->dim.h = e.xconfigure.height;
        ui_render(ui);
      }
    }
  }
}
