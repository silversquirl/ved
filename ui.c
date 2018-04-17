#include <stdlib.h>
#include <errno.h>
#include <cairo-xlib.h>
#include "ui_internal.h"

#define UI_TEXT_PADDING 5

#define LAYOUT_TEXT_LEN_INIT 512
static void ui_damage_buffer(void *uip) {
	struct ui *ui = uip;
	struct buffer *buf = &ui->ved->buffer;
	buf_view_init(buf, 0); // TODO: scrolling

	int height;
	do {
		buf_view_extend(buf);
		pango_layout_set_text(ui->text.l, buf->edit.buf, buf->edit.len);
		pango_layout_get_pixel_size(ui->text.l, NULL, &height);
	} while (height < ui->dim.h);
}

static inline void ui_set_colour(struct ui *ui, struct colour c) {
	cairo_set_source_rgb(ui->draw.cr, c.r, c.g, c.b);
}

static inline void ui_draw_at(struct ui *ui, int y) {
	cairo_move_to(ui->draw.cr, 0, y);
	ui_set_colour(ui, ui->colours.fg);
	pango_cairo_show_layout(ui->draw.cr, ui->text.l);
}

static void ui_render(struct ui *ui) {
	cairo_push_group(ui->draw.cr);

	cairo_rectangle(ui->draw.cr, -UI_TEXT_PADDING, 0, ui->dim.w, ui->dim.h);
	ui_set_colour(ui, ui->colours.bg);
	cairo_fill(ui->draw.cr);

	// TODO: configurable tabstops
	// TODO: alignment detection
	// TODO: scrolling
	ui_draw_at(ui, 0);

	cairo_pop_group_to_source(ui->draw.cr);
	cairo_paint(ui->draw.cr);
	cairo_surface_flush(ui->draw.surf);
	XFlush(ui->dpy);
}

static void ui_resize(struct ui *ui) {
	cairo_xlib_surface_set_size(ui->draw.surf, ui->dim.w, ui->dim.h);
	pango_cairo_update_layout(ui->draw.cr, ui->text.l);

	int w = PANGO_SCALE * (ui->dim.w - UI_TEXT_PADDING * 2);
	pango_layout_set_width(ui->text.l, w);

	ui->ved->buffer.damage_cb(ui->ved->buffer.damage_data);
}

static void ui_keypress(struct ui *ui, XKeyEvent xk) {
	cmd_handle_key(ui->ved->modes.current, ui, xk.keycode);
}

static void ui_keyrelease(struct ui *ui, XKeyEvent xk) {
	// TODO: key releases
}

struct ui *ui_init(struct editor *ved) {
	struct ui *ui = malloc(sizeof *ui);
	if (!ui) return NULL;
	ui->ved = ved;

	// X11
	ui->dpy = XOpenDisplay(NULL);
	if (!ui->dpy) {
		free(ui);
		return NULL;
	}
	int scr = DefaultScreen(ui->dpy);
	int black = BlackPixel(ui->dpy, scr);
	ui->w = XCreateSimpleWindow(ui->dpy, DefaultRootWindow(ui->dpy), 0, 0, 600, 400, 0, black, black);

	long events = 0;
	events |= StructureNotifyMask; // Tells us when the window resizes
	events |= ExposureMask; // Tells us when to redraw
	events |= KeyPressMask | KeyReleaseMask; // Keyboard events
	events |= KeymapStateMask; // Keyboard state on window entry
	XSelectInput(ui->dpy, ui->w, events);
	ui->exit = false;

	ui->atoms.wm_delete_window = XInternAtom(ui->dpy, "WM_DELETE_WINDOW", false);
	XSetWMProtocols(ui->dpy, ui->w, &ui->atoms.wm_delete_window, 1);

	// Colours
	ui->colours.fg = (struct colour){ 1, 1, 1 };
	ui->colours.bg = (struct colour){ 0, 0, 0 };

	// Cairo
	XWindowAttributes wattr;
	XGetWindowAttributes(ui->dpy, ui->w, &wattr);
	ui->draw.surf = cairo_xlib_surface_create(ui->dpy, ui->w, wattr.visual, ui->dim.w, ui->dim.h);
	ui->draw.cr = cairo_create(ui->draw.surf);
	cairo_translate(ui->draw.cr, UI_TEXT_PADDING, 0);

	// Pango
	ui->text.l = pango_cairo_create_layout(ui->draw.cr);
	pango_layout_set_wrap(ui->text.l, PANGO_WRAP_WORD_CHAR);

	PangoFontDescription *fdesc = pango_font_description_from_string("Helvetica 11");
	pango_layout_set_font_description(ui->text.l, fdesc);
	pango_font_description_free(fdesc);

	// Buffer drawing
	ui->ved->buffer.damage_cb = ui_damage_buffer;
	ui->ved->buffer.damage_data = ui;

	// Force a size configuration
	ui_resize(ui);

	return ui;
}

void ui_free(struct ui *ui) {
	g_object_unref(ui->text.l);

	cairo_surface_finish(ui->draw.surf);
	cairo_destroy(ui->draw.cr);
	cairo_surface_destroy(ui->draw.surf);

	XDestroyWindow(ui->dpy, ui->w);
	free(ui);
}

void ui_set_quit_cb(struct ui *ui, bool (*cb)(struct ui *)) {
	ui->quit_cb = cb;
}

void ui_quit(struct ui *ui) {
	if (!ui->quit_cb || ui->quit_cb(ui))
		ui->exit = true;
}

void ui_mainloop(struct ui *ui) {
	XMapWindow(ui->dpy, ui->w);
	XEvent e;
	while (!ui->exit) {
		XNextEvent(ui->dpy, &e);
		switch (e.type) {
		case Expose:
			ui_render(ui);
			break;

		case ConfigureNotify:
			if (e.xconfigure.width != ui->dim.w || e.xconfigure.height != ui->dim.h) {
				ui->dim.w = e.xconfigure.width;
				ui->dim.h = e.xconfigure.height;
				ui_resize(ui);
				ui_render(ui);
			}

		case MappingNotify:
			// Detect changes to the keyboard mapping
			switch (e.xmapping.request) {
			case MappingModifier:
			case MappingKeyboard:
				XRefreshKeyboardMapping(&e.xmapping);
				break;
			}

		case KeyPress:
			ui_keypress(ui, e.xkey);
			break;

		case KeyRelease:
			ui_keyrelease(ui, e.xkey);
			break;

		case ClientMessage:
			if (e.xclient.data.l[0] == ui->atoms.wm_delete_window)
				ui_quit(ui);
			break;
		}
	}
}
