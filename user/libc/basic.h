
#ifndef NG_LIBC_BASIC_H
#define NG_LIBC_BASIC_H

#include <stdint.h>

typedef signed long ssize_t;
typedef unsigned long size_t;

#ifdef __GNUC__
#define __unreachable __builtin_unreachable()
#define asm __asm__
#else
#error "Support non-GNUC attributes first"
#endif

static inline intptr_t max(intptr_t a, intptr_t b) {
        if (a > b)
                return a;
        else
                return b;
}

static inline intptr_t min(intptr_t a, intptr_t b) {
        if (a > b)
                return b;
        else
                return a;
}

static inline uintptr_t round_up(uintptr_t val, uintptr_t place) {
        return (val + place - 1) & ~(place - 1);
}

static inline uintptr_t round_down(uintptr_t val, uintptr_t place) {
        return val & ~(place - 1);
}

#endif
