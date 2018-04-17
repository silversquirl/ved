#ifndef __VED_UTIL_H__
#define __VED_UTIL_H__

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>

typedef unsigned long long biggest_int;

// Returns true if a * b will be greater than max
static inline bool will_overflow(biggest_int a, biggest_int b, biggest_int max) {
	return a && b > max / a;
}

static inline void *realloc_multiply(void *ptr, size_t nmemb, size_t size) {
	if (will_overflow(nmemb, size, (size_t)-1)) {
		errno = ENOMEM;
		return NULL;
	}
	return realloc(ptr, nmemb * size);
}

// Convenience wrapper
#define realloc_array(array, nmemb) (realloc_multiply(array, nmemb, sizeof *(array)))

#endif
