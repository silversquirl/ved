// The actual declaration of the ui struct
// It's in a separate header so we don't need to include Pango and Xlib
// everywhere that interacts with the UI

#include <pango/pangoxft.h>
#include <X11/Xlib.h>
#include "ui.h"

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
		PangoLayout *l1, *l2, *l3;
	} text;

	// Called when the UI attempts to quit to check if it's allowed to.
	// true means "yes, quit now", false means "no, don't quit yet"
	bool (*quit_cb)(struct ui *ui);
};
