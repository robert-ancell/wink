#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef uint32_t ref_t;

// Initialize reference count.
void ref_init(ref_t *ref);

// Increment reference count.
void ref_inc(ref_t *ref);

// Decrement reference count returning true if the object should be destroyed
bool ref_dec(ref_t *ref);
