#ifndef __VED_ROPE_H__
#define __VED_ROPE_H__

#include <stddef.h>
#include <stdio.h>

typedef char **rope;

rope make_rope(size_t n);
void free_rope(rope r);
size_t rope_len(rope r);
int rope_init(rope *r, ...); // segment1, segment2, ..., NULL
rope rope_dup(rope r);
int rope_fprint(rope r, FILE *f);
#define rope_print(r) rope_fprint(r, stdout)

#endif
