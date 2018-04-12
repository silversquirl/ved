#ifndef __VED_VEV_H__
#define __VED_VEV_H__

// Yay events!

struct event;

typedef void (*vev_handler)(void *context, void *userdata);

struct event *vev_create(void);
void vev_free(struct event *ev);
int vev_register(struct event *ev, vev_handler handler, void *userdata);
void vev_dispatch(struct event *ev, void *context);

#endif
