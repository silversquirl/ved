#ifndef __VED_UI_H__
#define __VED_UI_H__

#include <stdbool.h>
#include "editor.h"
#include "vev.h"

struct ui;
struct ui *ui_init(struct editor *ved);
void ui_mainloop(struct ui *ui);
void ui_quit(struct ui *ui);

#endif
