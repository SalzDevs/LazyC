#ifndef LAZY_H
#define LAZY_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
  LAZY_OK = 0,
  LAZY_ERR_NULL_OBJECT,
  LAZY_ERR_NULL_COMPUTE,
  LAZY_ERR_NULL_OUTPUT,
  LAZY_ERR_INVALID_OUTPUT_SIZE,
  LAZY_ERR_MUTEX_INIT,
  LAZY_ERR_COMPUTE,
  LAZY_ERR_MUTEX_LOCK,
  LAZY_ERR_MUTEX_UNLOCK,
  LAZY_ERR_MUTEX_DESTROY,
  LAZY_ERR_INVALID_STATE,
  LAZY_ERR_ALLOC
} LazyStatus;

typedef struct Lazy Lazy;

typedef LazyStatus (*LazyComputeFn)(void *ctx, void *out);

LazyStatus lazy_create(Lazy **lazy, LazyComputeFn compute, void *ctx, void *out, size_t out_size);
LazyStatus lazy_eval(Lazy *lazy);
LazyStatus lazy_reset(Lazy *lazy);
LazyStatus lazy_destroy(Lazy *lazy);
LazyStatus lazy_is_computed(const Lazy *lazy, bool *computed);

#endif
