// The actual declaration of the ui struct
// It's not in ui.h so we don't need to include a bunch of unnecessary
// headers in every file that interacts with the UI

#include <pango/pangocairo.h>
#include <X11/Xlib.h>
#include <vtk.h>
#include "ui.h"

struct colour { double r, g, b, a; };

struct ui {
	struct editor *ved;

	vtk root;
	vtk_window win;
	cairo_t *cr;

	struct {
		struct colour
			fg,	// Text foreground
			bg,	// Window background
			esof;	// End/start of file line foreground
	} colours;

	struct {
		PangoLayout *l;
		double scroll, line_height;
	} text;

	// Called when the UI attempts to quit to check if it's allowed to.
	// true means "yes, quit now", false means "no, don't quit yet"
	bool (*quit_cb)(struct ui *ui);
};
