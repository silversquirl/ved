#ifndef __VED_ROPE_H__
#define __VED_ROPE_H__

/* Rope
 *
 * A rope is a way of storing strings for fast insertion and deletion.
 * Ved uses ropes for storing file content, as they allow fast editing and
 * copying of strings. The copying aspect is important, as Ved's IO thread
 * will need a copy of the input buffer that is not modified while it is
 * writing. Ropes provide an efficient way of doing this, as the segments
 * of a rope are not modified when the rope is modified.
 *
 * Ved's rope is implemented very differently to a typical rope data
 * structure, almost to the point of it not being classifiable as a rope.
 * Where the official (read: Wikipedia) definition of a rope involves a
 * binary tree, Ved's rope uses a simple vector. This makes it much
 * simpler to access and manipulate using normal C methods such as array
 * indexing.
 *
 * The technique of storing metadata before the returned pointer was
 * inspired by antirez's sds library and stb's stretchy_buffer.h
 */

#include <stddef.h>
#include <stdio.h>
#include "vev.h"

struct rope_item {
	char *s;
	size_t len;
};
typedef struct rope_item *rope;

rope make_rope(size_t n);
void free_rope(rope r);
size_t rope_len(rope r);
int rope_init(rope *r, ...); // segment1, segment2, ..., NULL
rope rope_dup(rope r);
int rope_fprint(rope r, FILE *f);
#define rope_print(r) rope_fprint(r, stdout)
char *rope_flatten(rope r, size_t *len); // length is put in len (if not NULL)

struct rope_events {
	struct event *update;
};
struct rope_events rope_events(rope r);

#endif
