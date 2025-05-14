#include "ref.h"

void ref_init(ref_t *ref) { *ref = 0; }

void ref_inc(ref_t *ref) { (*ref)++; }

bool ref_dec(ref_t *ref) {
  if (*ref == 0) {
    return 0;
  }

  (*ref)--;
  return *ref == 0;
}
