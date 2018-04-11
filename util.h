#ifndef __VED_UTIL_H__
#define __VED_UTIL_H__

#include <stdbool.h>

typedef unsigned long long biggest_int;

// Returns true if a * b will be greater than max
static inline bool will_overflow(biggest_int a, biggest_int b, biggest_int max) {
  return a && b > max / a;
}

#endif
