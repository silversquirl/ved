#ifndef __VED_UI_H__
#define __VED_UI_H__

#include <stdbool.h>
#include "editor.h"

struct ui;
struct ui *ui_init(struct editor *ved);
void ui_free(struct ui *ui);
void ui_set_quit_cb(struct ui *ui, bool (*cb)(struct ui *));
void ui_quit(struct ui *ui);
void ui_mainloop(struct ui *ui);

#endif
