#ifndef __VED_UI_H__
#define __VED_UI_H__

#include <pango/pangoxft.h>
#include <X11/Xlib.h>
#include "editor.h"

struct ui {
  struct editor *ved;

  Display *dpy;
  Window w;
  struct { unsigned w, h; } dim;

  struct {
    XftDraw *draw;
    XftColor fg;
    PangoLayout *layout;
  } text;
};

int ui_init(struct ui *ui, struct editor *ved);
void ui_mainloop(struct ui *ui);

#endif
