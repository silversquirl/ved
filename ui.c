#include <stdlib.h>
#include <errno.h>
#include <cairo-xlib.h>
#include <X11/extensions/XInput2.h>
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

static void ui_render(struct ui *ui) {
	cairo_push_group(ui->draw.cr);

	cairo_translate(ui->draw.cr, 0, 0);
	cairo_rectangle(ui->draw.cr, 0, 0, ui->dim.w, ui->dim.h);
	ui_set_colour(ui, ui->colours.bg);
	cairo_fill(ui->draw.cr);

	cairo_translate(ui->draw.cr, 0, ui->text.scroll);

	cairo_move_to(ui->draw.cr, UI_TEXT_PADDING, 0);
	cairo_line_to(ui->draw.cr, ui->dim.w - UI_TEXT_PADDING * 2, 0);
	ui_set_colour(ui, ui->colours.fg);
	cairo_set_line_width(ui->draw.cr, 1);
	cairo_stroke(ui->draw.cr);

	// TODO: configurable indentation
	// TODO: elastic tabstops (see http://nickgravgaard.com/elastic-tabstops/)
	cairo_move_to(ui->draw.cr, UI_TEXT_PADDING, 0);
	ui_set_colour(ui, ui->colours.fg);
	pango_cairo_show_layout(ui->draw.cr, ui->text.l);

	cairo_pop_group_to_source(ui->draw.cr);
	cairo_paint(ui->draw.cr);
	cairo_surface_flush(ui->draw.surf);
	XFlush(ui->dpy);
}

static void ui_scroll(struct ui *ui, double delta) {
	ui->text.scroll += delta * ui->text.scroll_factor;
	ui_render(ui);
}

static void ui_resize(struct ui *ui) {
	cairo_xlib_surface_set_size(ui->draw.surf, ui->dim.w, ui->dim.h);
	pango_cairo_update_layout(ui->draw.cr, ui->text.l);

	int w = PANGO_SCALE * (ui->dim.w - UI_TEXT_PADDING * 2);
	pango_layout_set_width(ui->text.l, w);

	ui->ved->buffer.damage_cb(ui->ved->buffer.damage_data);
}

// True means scroll valuator was found, false means not
static bool ui_update_xi2_scroll(struct ui *ui, XIAnyClassInfo **classes, int nclass) {
	for (int i = 0; i < nclass; ++i) {
		XIScrollClassInfo *scroll = (XIScrollClassInfo *)classes[i];
		if (scroll->type != XIScrollClass) continue;
		// TODO: consider supporting horizontal scrolling
		if (scroll->scroll_type != XIScrollTypeVertical) continue;

		ui->input.scroll_v.valuator = scroll->number;
		ui->input.scroll_v.increment = scroll->increment;

		// Change of valuators means we need to reset the scroll value
		ui->input.scroll_v.reset = true;

		return true;
	}
	return false;
}

static void ui_keypress(struct ui *ui, XKeyEvent xk) {
	cmd_handle_key(ui->ved->modes.current, ui, xk.keycode);
}

static void ui_keyrelease(struct ui *ui, XKeyEvent xk) {
	// TODO: key releases
}

static void ui_buttonpress(struct ui *ui, XButtonEvent xb) {
	switch (xb.button) {
	case 4:
		if (!ui->input.use_xi2)
			ui_scroll(ui, +1);
		break;
	case 5:
		if (!ui->input.use_xi2)
			ui_scroll(ui, -1);
		break;
	case 6:
	case 7:
		// TODO: consider supporting horizontal scrolling
		break;

	default:
		printf("Button %d\n", xb.button);
		break;
	}
}

static void ui_xinput(struct ui *ui, XGenericEventCookie xc) {
	XIDeviceChangedEvent *dc;
	XIDeviceEvent *de;
	XIEnterEvent *ee;

	switch (xc.evtype) {
	case XI_DeviceChanged:
		dc = xc.data;
		// Ignore return because we don't care if it's not a scroll valuator change
		ui_update_xi2_scroll(ui, dc->classes, dc->num_classes);
		break;

	case XI_Motion:
		de = xc.data;
		for (int b = 0, i = 0; b < de->valuators.mask_len; ++b) {
			for (int n = 0; de->valuators.mask[b] >> n; ++n) {
				if ((de->valuators.mask[b] >> n) & 1) {
					int bit = b * CHAR_BIT + n;
					if (bit == ui->input.scroll_v.valuator) {
						double val = de->valuators.values[i];
						if (ui->input.scroll_v.reset) {
							ui->input.scroll_v.reset = false;
						} else {
							double delta = ui->input.scroll_v.val - val;
							ui_scroll(ui, delta / ui->input.scroll_v.increment);
						}
						ui->input.scroll_v.val = val;
					}
					++i;
				}
			}
		}
		break;

	case XI_Enter:
		ee = xc.data;
		if (ee->evtype == XI_Enter && ee->mode == XINotifyNormal)
			ui->input.scroll_v.reset = true;
		break;
	}
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
	events |= ButtonPressMask | ButtonReleaseMask; // Button events
	XSelectInput(ui->dpy, ui->w, events);
	ui->exit = false;

	ui->atoms.wm_delete_window = XInternAtom(ui->dpy, "WM_DELETE_WINDOW", false);
	XSetWMProtocols(ui->dpy, ui->w, &ui->atoms.wm_delete_window, 1);

	// Scrolling
	// FIXME: This is a mess of ugly nested ifs. Refactor to use a ui_init_xi2 function
	ui->input.scroll_v.reset = true;
	ui->input.use_xi2 = false;
	ui->text.scroll = 0.0;

	int event, error, xi_maj = 2, xi_min = 1;
	if (!XQueryExtension(ui->dpy, "XInputExtension", &ui->input.opcode, &event, &error)
			|| XIQueryVersion(ui->dpy, &xi_maj, &xi_min) == BadRequest) {
		fprintf(stderr, "Error initialising XInput2. Falling back to legacy mouse button scrolling.\n");
	} else {
		int xi_ndev;
		XIDeviceInfo *xi_info = XIQueryDevice(ui->dpy, XIAllMasterDevices, &xi_ndev);
		for (int i = 0; i < xi_ndev; ++i) {
			if (ui_update_xi2_scroll(ui, xi_info[i].classes, xi_info[i].num_classes)) {
				ui->input.device = xi_info[i].deviceid;
				ui->input.use_xi2 = true;
				break;
			}
		}

		if (ui->input.use_xi2) {
			XIEventMask xi_emask = {
				.deviceid = ui->input.device,
				.mask_len = 1,
				.mask = (unsigned char [1]){ 0 },
			};
			XISetMask(xi_emask.mask, XI_DeviceChanged);
			XISetMask(xi_emask.mask, XI_Motion);
			XISetMask(xi_emask.mask, XI_Enter);
			XISelectEvents(ui->dpy, ui->w, &xi_emask, 1);
		} else {
			fprintf(stderr, "Could not find vertical scroll valuator with XInput2. Falling back to legacy mouse button scrolling.\n");
		}
	}

	// Colours
	ui->colours.fg = (struct colour){ 1, 1, 1 };
	ui->colours.bg = (struct colour){ 0, 0, 0 };

	// Cairo
	XWindowAttributes wattr;
	XGetWindowAttributes(ui->dpy, ui->w, &wattr);
	ui->draw.surf = cairo_xlib_surface_create(ui->dpy, ui->w, wattr.visual, ui->dim.w, ui->dim.h);
	ui->draw.cr = cairo_create(ui->draw.surf);
	cairo_translate(ui->draw.cr, 0, 0);

	// Pango
	ui->text.l = pango_cairo_create_layout(ui->draw.cr);
	pango_layout_set_wrap(ui->text.l, PANGO_WRAP_WORD_CHAR);

	PangoFontDescription *fdesc = pango_font_description_from_string("Helvetica 11");
	pango_layout_set_font_description(ui->text.l, fdesc);

	PangoFont *font = pango_font_map_load_font(
		pango_cairo_font_map_get_default(),
		pango_layout_get_context(ui->text.l),
		fdesc);
	PangoFontMetrics *fmetrics = pango_font_get_metrics(font, NULL);
	int asc = pango_font_metrics_get_ascent(fmetrics);
	int desc = pango_font_metrics_get_descent(fmetrics);
	ui->text.scroll_factor = (asc + desc) / (double)PANGO_SCALE * 1.5;
	pango_font_metrics_unref(fmetrics);

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

		case ButtonPress:
			ui_buttonpress(ui, e.xbutton);
			break;

		case GenericEvent:
			if (e.xgeneric.extension == ui->input.opcode && XGetEventData(ui->dpy, &e.xcookie)) {
				ui_xinput(ui, e.xcookie);
				XFreeEventData(ui->dpy, &e.xcookie);
			}
			break;

		case ClientMessage:
			if (e.xclient.data.l[0] == ui->atoms.wm_delete_window)
				ui_quit(ui);
			break;
		}
	}
}
