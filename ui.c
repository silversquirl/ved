#include <errno.h>
#include "ui.h"
#include "ui_internal.h"

#define UI_TEXT_BORDER 5

static int ui_colour(struct ui *ui, XftColor *c, const char *name) {
	Visual *visual = XftDrawVisual(ui->text.draw);
	Colormap cmap = XftDrawColormap(ui->text.draw);
	return !XftColorAllocName(ui->dpy, visual, cmap, name, c);
}

#define LAYOUT_TEXT_LEN_INIT 512
// Returns -1 if the window is full, the height of the pango layout otherwise
static inline int ui_load_from(struct ui *ui, PangoLayout *l, const char *buf, size_t len) {
	const int win_height_pango = ui->dim.h * PANGO_SCALE;
	int height;
	size_t n = LAYOUT_TEXT_LEN_INIT;

	do {
		pango_layout_set_text(l, buf, n < len ? n : len);
		if (n >= len) break;
		pango_layout_get_size(l, NULL, &height);
		n *= 2;
	} while (height < win_height_pango);

	if (height < win_height_pango) return height;
	else return 0;
}

static inline void ui_draw_at(struct ui *ui, PangoLayout *l, int y) {
	pango_xft_render_layout(ui->text.draw, &ui->text.fg, l, UI_TEXT_BORDER * PANGO_SCALE, y);
}

static inline void ui_draw_text(struct ui *ui) {
	const struct buffer b = ui->ved->buffer;

	// TODO: only recalculate layouts if they're invalid (buffer has changed
	//       or window has resized and we need more data)
	// TODO: configurable tabstops
	// TODO: alignment detection

	if (b.edit.len) {
		int h1, h2;
		h1 = ui_load_from(ui, ui->text.l1, b.file.buf, b.edit.start);
		ui_draw_at(ui, ui->text.l1, 0);
		if (!h1) return;

		h2 = ui_load_from(ui, ui->text.l2, b.edit.buf, b.edit.len);
		ui_draw_at(ui, ui->text.l2, h1);
		if (!h2) return;

		ui_load_from(ui, ui->text.l3, b.file.buf + b.edit.end, b.file.len - b.edit.end);
		ui_draw_at(ui, ui->text.l3, h1 + h2);
	} else {
		ui_load_from(ui, ui->text.l1, b.file.buf, b.file.len);
		ui_draw_at(ui, ui->text.l1, 0);
	}
}

static void ui_render(struct ui *ui) {
	// TODO: double buffering
	XClearWindow(ui->dpy, ui->w);
	ui_draw_text(ui);
	XFlush(ui->dpy);
}

static void ui_resize(struct ui *ui) {
	int w = ui->dim.w * PANGO_SCALE - UI_TEXT_BORDER * PANGO_SCALE * 2;
	pango_layout_set_width(ui->text.l1, w);
	pango_layout_set_width(ui->text.l2, w);
	pango_layout_set_width(ui->text.l3, w);
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

	// Pango
	PangoFontMap *fm  = pango_xft_get_font_map(ui->dpy, scr);
	PangoContext *ctx = pango_font_map_create_context(fm);
	ui->text.l1 = pango_layout_new(ctx);
	ui->text.l2 = pango_layout_new(ctx);
	ui->text.l3 = pango_layout_new(ctx);
	pango_layout_set_wrap(ui->text.l1, PANGO_WRAP_WORD_CHAR);
	pango_layout_set_wrap(ui->text.l2, PANGO_WRAP_WORD_CHAR);
	pango_layout_set_wrap(ui->text.l3, PANGO_WRAP_WORD_CHAR);

	PangoFontDescription *fdesc = pango_font_description_from_string("Helvetica 11");
	pango_layout_set_font_description(ui->text.l1, fdesc);
	pango_layout_set_font_description(ui->text.l2, fdesc);
	pango_layout_set_font_description(ui->text.l3, fdesc);
	pango_font_description_free(fdesc);

	// Xft
	Visual *v = DefaultVisual(ui->dpy, scr);
	Colormap cmap = DefaultColormap(ui->dpy, scr);
	ui->text.draw = XftDrawCreate(ui->dpy, ui->w, v, cmap);
	ui_colour(ui, &ui->text.fg, "white");

	// vev
	ui->ev.keypress = vev_create();
	ui->ev.keyrelease = vev_create();
	ui->ev.quit = vev_create();

	return ui;
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
			vev_dispatch(ui->ev.keypress, &e.xkey);
			break;

		case KeyRelease:
			vev_dispatch(ui->ev.keyrelease, &e.xkey);
			break;

		case ClientMessage:
			if (e.xclient.data.l[0] == ui->atoms.wm_delete_window) {
				ui->exit = true;
				vev_dispatch(ui->ev.quit, &e.xclient);
			}
			break;
		}
	}
	XDestroyWindow(ui->dpy, ui->w);
}

void ui_quit(struct ui *ui) {
	ui->exit = true;
}
