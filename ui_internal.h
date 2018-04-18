// The actual declaration of the ui struct
// It's not in ui.h so we don't need to include a bunch of unnecessary
// headers in every file that interacts with the UI

#include <pango/pangocairo.h>
#include <X11/Xlib.h>
#include "ui.h"

struct colour { double r, g, b; };

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
		bool use_xi2;
		int opcode, device;

		struct {
			int valuator;
			double increment;

			bool reset;
			double val;
		} scroll_v;
	} input;

	struct {
		struct colour fg, bg;
	} colours;

	struct {
		cairo_surface_t *surf;
		cairo_t *cr;
	} draw;

	struct {
		PangoLayout *l;
		double scroll;
	} text;

	// Called when the UI attempts to quit to check if it's allowed to.
	// true means "yes, quit now", false means "no, don't quit yet"
	bool (*quit_cb)(struct ui *ui);
};
