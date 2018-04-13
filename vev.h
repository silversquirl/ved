#ifndef __VED_VEV_H__
#define __VED_VEV_H__

// Yay events!

#include <errno.h>

struct event;

typedef void (*vev_handler)(void *context, void *userdata);

struct event *vev_create(void);
void vev_free(struct event *ev);
// If this errors, errno will be nonzero
void vev_register_wrapped(struct event *ev, vev_handler handler, void *userdata);
void vev_dispatch(struct event *ev, void *context);

// void vev_register(struct event *ev, <function pointer that accepts 2 pointer args> handler, void *userdata);
#define vev_register(ev, handler, userdata) do { void __vev_wrapper__##handler(void *ctx, void *ud) { handler(ctx, ud); } vev_register_wrapped(ev, __vev_wrapper__##handler, userdata); } while (0);

#endif
