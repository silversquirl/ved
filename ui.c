#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <vtk.h>
#include "ui_internal.h"

#define UI_TEXT_PADDING 5

static inline int ui_text_height(struct ui *ui) {
	int height;
	pango_layout_get_pixel_size(ui->text.l, NULL, &height);
	return height;
}

static inline bool ui_view_at_eof(struct ui *ui) {
	return ui->ved->buffer.file.len == ui->ved->buffer.edit.end;
}

static inline int ui_height_target(struct ui *ui) {
	int h;
	vtk_window_get_size(ui->win, NULL, &h);
	return h - ui->text.scroll + 1;
}

static void ui_damage_buffer(void *uip) {
	struct ui *ui = uip;
	struct buffer *buf = &ui->ved->buffer;
	bool eof = false;

	const int target = ui_height_target(ui);
	do {
		switch (buf_view_extend(buf)) {
		case 0: // Success
			break;
		case -1: // Error
			perror("view_extend");
			break;
		case 1: // EOF reached
			eof = true;
			break;
		}
		pango_layout_set_text(ui->text.l, buf->edit.buf, buf->edit.len);
	} while (!eof && ui_text_height(ui) < target);
}

static bool ui_shrink_buffer(struct ui *ui) {
	struct buffer *buf = &ui->ved->buffer;

	const int target = ui_height_target(ui);
	size_t decrement = 0;
	while (ui_text_height(ui) > target) {
		decrement += EDIT_ALLOC_STEP;
		pango_layout_set_text(ui->text.l, buf->edit.buf, buf->edit.len - decrement);
	}

	if (!decrement)
		return false;

	if (buf_view_shrink(buf, decrement - EDIT_ALLOC_STEP) == -1)
		perror("view_shrink");

	return true;
}

static inline void ui_set_colour(struct ui *ui, struct colour c) {
	cairo_set_source_rgba(ui->cr, c.r, c.g, c.b, c.a);
}

static void ui_set_pango_size(struct ui *ui) {
	int w;
	vtk_window_get_size(ui->win, &w, NULL);
	w -= UI_TEXT_PADDING * 2;
	w *= PANGO_SCALE;
	pango_layout_set_width(ui->text.l, w);
}

static void ui_close(vtk_event ev, void *u) {
	struct ui *ui = u;
	ui_quit(ui);
}

static void ui_draw(vtk_event ev, void *u) {
	struct ui *ui = u;
	int w, h;
	vtk_window_get_size(ui->win, &w, &h);

	cairo_translate(ui->cr, 0, 0);
	cairo_rectangle(ui->cr, 0, 0, w, h);
	ui_set_colour(ui, ui->colours.bg);
	cairo_fill(ui->cr);

	cairo_translate(ui->cr, 0, ui->text.scroll);

	// Draw lines above and below the text
	ui_set_colour(ui, ui->colours.esof);
	cairo_set_line_width(ui->cr, 1);
	const double sx = UI_TEXT_PADDING, ex = w - UI_TEXT_PADDING * 2;

	cairo_move_to(ui->cr, sx, 0);
	cairo_line_to(ui->cr, ex, 0);
	cairo_stroke(ui->cr);

	const double y = ui_text_height(ui);
	cairo_move_to(ui->cr, sx, y);
	cairo_line_to(ui->cr, ex, y);
	cairo_stroke(ui->cr);

	// TODO: configurable indentation
	// TODO: elastic tabstops (see http://nickgravgaard.com/elastic-tabstops/)
	cairo_move_to(ui->cr, UI_TEXT_PADDING, 0);
	ui_set_colour(ui, ui->colours.fg);
	pango_cairo_show_layout(ui->cr, ui->text.l);
}

static void ui_keypress(vtk_event ev, void *u) {
	struct ui *ui = u;
	cmd_handle_key(ui->ved->modes.current, ui, ev.key.key);
}

static void ui_resize(vtk_event ev, void *u) {
	struct ui *ui = u;
	pango_cairo_update_layout(ui->cr, ui->text.l);
	ui_set_pango_size(ui);
	ui_damage_buffer(ui);
}

static void ui_scroll(vtk_event ev, void *u) {
	struct ui *ui = u;
	ui->text.scroll += ev.scroll.amount * ui->text.line_height * 1.5;

	int h;
	vtk_window_get_size(ui->win, NULL, &h);

	double scroll_max = h / 2.;
	if (ui->text.scroll > scroll_max)
		ui->text.scroll = scroll_max;

	if (ui_view_at_eof(ui)) {
		double scroll_min = scroll_max - ui_text_height(ui);
		if (ui->text.scroll < scroll_min)
			ui->text.scroll = scroll_min;
	}

	if (ui_text_height(ui) > h - ui->text.scroll + 1)
		ui_shrink_buffer(ui);
	ui_damage_buffer(ui);
	vtk_window_redraw(ui->win);
}

struct ui *ui_init(struct editor *ved) {
	struct ui *ui = malloc(sizeof *ui);
	if (!ui) return NULL;
	ui->ved = ved;

	// vtk
	int err;
	if ((err = vtk_new(&ui->root))) {
		fprintf(stderr, "Error creating vtk root: %s\n", vtk_strerr(err));
		return NULL;
	}
	if ((err = vtk_window_new(&ui->win, ui->root, "ved", 0, 0, 800, 600))) {
		fprintf(stderr, "Error creating vtk window: %s\n", vtk_strerr(err));
		return NULL;
	}

	vtk_window_set_event_handler(ui->win, VTK_EV_CLOSE, ui_close, ui);
	vtk_window_set_event_handler(ui->win, VTK_EV_DRAW, ui_draw, ui);
	vtk_window_set_event_handler(ui->win, VTK_EV_KEY_PRESS, ui_keypress, ui);
	vtk_window_set_event_handler(ui->win, VTK_EV_RESIZE, ui_resize, ui);
	vtk_window_set_event_handler(ui->win, VTK_EV_SCROLL, ui_scroll, ui);

	// Colours
	ui->colours.fg	= (struct colour){ 1, 1, 1, 1 };
	ui->colours.bg	= (struct colour){ 0, 0, 0, .6 };
	ui->colours.esof	= (struct colour){ .5, .5, .5, 1 };

	// Cairo
	ui->cr = vtk_window_get_cairo(ui->win);
	cairo_translate(ui->cr, 0, 0);

	// Pango
	ui->text.l = pango_cairo_create_layout(ui->cr);
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
	ui->text.line_height = (asc + desc) / (double)PANGO_SCALE;
	pango_font_metrics_unref(fmetrics);

	pango_font_description_free(fdesc);

	ui_set_pango_size(ui);

	// Buffer drawing
	ui->ved->buffer.damage_cb = ui_damage_buffer;
	ui->ved->buffer.damage_data = ui;

	return ui;
}

void ui_free(struct ui *ui) {
	g_object_unref(ui->text.l);
	vtk_window_destroy(ui->win);
	vtk_destroy(ui->root);
	free(ui);
}

void ui_set_quit_cb(struct ui *ui, bool (*cb)(struct ui *)) {
	ui->quit_cb = cb;
}

void ui_quit(struct ui *ui) {
	if (!ui->quit_cb || ui->quit_cb(ui)) vtk_window_close(ui->win);
}

void ui_mainloop(struct ui *ui) {
	vtk_window_mainloop(ui->win);
}
