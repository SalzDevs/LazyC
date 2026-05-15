#ifndef LAZY_H
#define LAZY_H

#include <stdbool.h>
#include <stddef.h>

/**
 * Status codes returned by the lazy API.
 */
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

/**
 * Opaque lazy object.
 *
 * Create with lazy_create() and release with lazy_destroy().
 */
typedef struct Lazy Lazy;

/**
 * Computes a lazy value into caller-owned output storage.
 *
 * ctx may be NULL. out is the same pointer passed to lazy_create().
 * Return LAZY_OK only when the output has been successfully written.
 */
typedef LazyStatus (*LazyComputeFn)(void *ctx, void *out);

/**
 * Create a lazy object.
 *
 * The caller owns the output storage passed as out. That storage must remain
 * valid until lazy_destroy() is called. The lazy object does not allocate,
 * clear, copy, or free the output value.
 *
 * ctx may be NULL. compute and out must not be NULL. out_size must be > 0.
 *
 * On failure, if lazy is not NULL, *lazy is set to NULL.
 */
LazyStatus lazy_create(Lazy **lazy, LazyComputeFn compute, void *ctx, void *out, size_t out_size);

/**
 * Evaluate the lazy value if it has not been computed yet.
 *
 * On success, the caller-owned output storage contains the computed value.
 * If the value was already computed, this is a cache hit and compute is not
 * called again.
 */
LazyStatus lazy_eval(Lazy *lazy);

/**
 * Mark the lazy value as not computed.
 *
 * This invalidates the cached state only. It does not clear or modify the
 * caller-owned output storage, which may still contain stale data until the
 * next successful lazy_eval().
 */
LazyStatus lazy_reset(Lazy *lazy);

/**
 * Destroy a lazy object created by lazy_create().
 *
 * The caller must ensure no other thread is using the same Lazy object while
 * lazy_destroy() is running. The caller-owned output storage is not freed.
 */
LazyStatus lazy_destroy(Lazy *lazy);

/**
 * Query whether the lazy value is currently computed.
 */
LazyStatus lazy_is_computed(Lazy *lazy, bool *computed);

#endif
