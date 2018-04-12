#include <stdlib.h>
#include "vev.h"

struct evhandler {
	vev_handler h;
	void *d;
	struct evhandler *next;
};

struct event {
	struct evhandler *first, *last;
};

struct event *vev_create(void) {
	struct event *ret = malloc(sizeof *ret);
	ret->first = NULL;
	ret->last = NULL;
	return ret;
}

void vev_free(struct event *ev) {
	struct evhandler *h = ev->first, *next;
	while (h) {
		next = h->next;
		free(h);
		h = next;
	}
	free(ev);
}

int vev_register(struct event *ev, vev_handler handler, void *userdata) {
	struct evhandler *h = malloc(sizeof *h);
	if (!h) return -1;
	h->h = handler;
	h->d = userdata;
	h->next = NULL;
	if (ev->last)
		ev->last->next = h;
	else
		ev->first = h;
	ev->last = h;
	return 0;
}

void vev_dispatch(struct event *ev, void *context) {
	struct evhandler *h;
	for (h = ev->first; h; h = h->next)
		h->h(context, h->d);
}
