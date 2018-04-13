#ifndef __VED_UI_H__
#define __VED_UI_H__

#include <stdbool.h>
#include <pango/pangoxft.h>
#include <X11/Xlib.h>
#include "editor.h"
#include "vev.h"

struct ui {
	struct editor *ved;

	Display *dpy;
	Window w;
	struct { unsigned w, h; } dim;
	bool exit;

	struct {
		Atom wm_delete_window;
	} atoms;

	struct {
		XftDraw *draw;
		XftColor fg;
		PangoLayout *layout;
	} text;

	struct {
		struct event *keypress;
		struct event *keyrelease;
		struct event *quit;
	} ev;
};

struct ui *ui_init(struct editor *ved);
void ui_mainloop(struct ui *ui);
void ui_quit(struct ui *ui);

#endif
