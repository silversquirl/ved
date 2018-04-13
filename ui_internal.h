// The actual declaration of the ui struct
// It's in a separate header so we don't need to include Pango and Xlib
// everywhere that interacts with the UI

#include <pango/pangoxft.h>
#include <X11/Xlib.h>

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
