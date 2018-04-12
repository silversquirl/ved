#include <errno.h>
#include <stdint.h>
#include <pango/pangoxft.h>
#include <X11/Xlib.h>
#include "ui.h"
#include "util.h"

struct action {
	KeySym key;
	void (*press)(void *);
	void (*release)(void *);
	void *data;
};

struct ui {
	struct editor *ved;

	Display *dpy;
	Window w;
	struct { unsigned w, h; } dim;
	bool exit;

	struct {
		Atom wm_delete_window;
	} atoms;

	struct action *actions; // Terminated by XK_VoidSymbol

	struct {
		XftDraw *draw;
		XftColor fg;
		PangoLayout *layout;
	} text;
};

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

static void ui_handle_keypress(struct ui *ui, XKeyEvent e) {
	for (struct action *a = ui->actions; a->key != XK_VoidSymbol; ++a)
		if (XKeysymToKeycode(ui->dpy, a->key) == e.keycode && a->press)
			return a->press(a->data);
}

static void ui_handle_keyrelease(struct ui *ui, XKeyEvent e) {
	for (struct action *a = ui->actions; a->key != XK_VoidSymbol; ++a)
		if (XKeysymToKeycode(ui->dpy, a->key) == e.keycode && a->release)
			return a->release(a->data);
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

	ui->actions = malloc(sizeof *ui->actions);
	if (!ui->actions) {
		free(ui);
		return NULL;
	}
	ui->actions[0] = (struct action){ XK_VoidSymbol };

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
			ui_handle_keypress(ui, e.xkey);
			break;

		case KeyRelease:
			ui_handle_keyrelease(ui, e.xkey);
			break;

		case ClientMessage:
			if (e.xclient.data.l[0] == ui->atoms.wm_delete_window)
				// Window closed
				// TODO: have some shut down code here
				ui->exit = true;
			break;
		}
	}
	XDestroyWindow(ui->dpy, ui->w);
}

int ui_add_action(struct ui *ui, char *key_name, void (*press)(void *), void (*release)(void *), void *data) {
	size_t l = 0;
	for (struct action *a = ui->actions; a->key != XK_VoidSymbol; ++a) ++l;

	if (will_overflow(l + 2, sizeof *ui->actions, SIZE_MAX)) {
		errno = ENOMEM;
		return -1;
	}
	struct action *a = realloc(ui->actions, (l + 2) * sizeof *ui->actions);
	if (!a) return -1;
	ui->actions = a;

	KeySym key_sym = XStringToKeysym(key_name);
	ui->actions[l] = (struct action){ key_sym, press, release, data };
	ui->actions[l + 1] = (struct action){ XK_VoidSymbol };
	return 0;
}

void ui_quit(struct ui *ui) {
	ui->exit = true;
}
