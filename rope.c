#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "rope.h"
#include "util.h"

struct rope_attr {
	size_t n, alloc;
};

// Returns true if a rope with n sections will overflow a size_t
static inline bool _rope_will_overflow(size_t n) {
	return will_overflow(n, sizeof (struct rope_attr), SIZE_MAX - sizeof (struct rope_attr));
}

// Returns a pointer to the rope attributes. This can also be used to get
// the start of the allocated buffer for the rope.
static inline struct rope_attr *_rope_attr(rope r) {
	if (!r) return NULL;
	return (struct rope_attr *)r - 1;
}

// Calculates the amount of memory requried for a rope of length n
static inline size_t _rope_alloclen(size_t n) {
	return n * sizeof (struct rope_item) + sizeof (struct rope_attr);
}

// Reallocates, taking into account the attributes at the start
static rope _rope_realloc(rope r, size_t n) {
	if (_rope_will_overflow(n)) {
		errno = ENOMEM;
		return NULL;
	}
	struct rope_attr *a = realloc(_rope_attr(r), _rope_alloclen(n));
	if (!a) return NULL;
	a->alloc = n;
	return (rope)(a + 1);
}

// Create an uninitialised rope with allocated length n
rope make_rope(size_t n) {
	if (_rope_will_overflow(n)) {
		errno = ENOMEM;
		return NULL;
	}
	rope r = _rope_realloc(NULL, n);
	if (!r) return NULL;

	struct rope_attr *a = _rope_attr(r);
	a->n = 0;
	return r;
}

size_t rope_len(rope r) {
	return _rope_attr(r)->n;
}

void free_rope(rope r) {
	free(_rope_attr(r));
}

int rope_init(rope *rptr, ...) { // segment1, segment2, ..., NULL
	rope r = *rptr;
	struct rope_attr *a = _rope_attr(r);
	va_list args;
	va_start(args, rptr);

	char *seg;
	size_t i = 0;
	while ((seg = va_arg(args, char *))) {
		if (i >= a->alloc) {
			r = _rope_realloc(r, a->alloc * 2);
			if (!r) return -1;
			a = _rope_attr(r);
		}

		r[i].s = seg;
		r[i].len = strlen(seg);
		++i;
	}
	a->n = i;
	*rptr = r;

	return 0;
}

rope rope_dup(rope r) {
	struct rope_attr *a = _rope_attr(r);
	rope new = make_rope(a->n);
	if (!new) return NULL;
	// This won't overflow because that was checked in make_rope
	memcpy(new, r, a->n * sizeof *new);
	return new;
}

int rope_fprint(rope r, FILE *f) {
	size_t len = rope_len(r);
	for (size_t i = 0; i < len; ++i) {
		if (fputs(r[i].s, f) == EOF) return -1;
	}
	return 0;
}

char *rope_flatten(rope r, size_t *len) {
	size_t l = 0, al = 32, rl = rope_len(r);
	char *buf = malloc(al);
	for (size_t i = 0; i < rl; ++i) {
		while (l + r[i].len >= al) {
			buf = realloc(buf, al *= 2);
			if (!buf) return NULL;
		}
		memcpy(buf + l, r[i].s, r[i].len);
		l += r[i].len;
	}
	if (al != l) buf = realloc(buf, l);
	if (len) *len = l;
	return buf;
}
