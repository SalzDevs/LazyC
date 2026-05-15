#include "lazy.h"

#include <pthread.h>
#include <stdlib.h>

typedef enum {
  LAZY_STATE_UNINITIALIZED = 0,
  LAZY_STATE_READY,
  LAZY_STATE_COMPUTED,
  LAZY_STATE_DESTROYED
} LazyState;

struct Lazy {
  LazyComputeFn compute;
  LazyState state;
  void *ctx;
  void *out;
  size_t out_size;
  pthread_mutex_t lock;
};

static LazyStatus validateLazyCreateArgs(Lazy **lazy, LazyComputeFn compute, void *out, size_t out_size) {
  if (lazy == NULL) return LAZY_ERR_NULL_OBJECT;
  if (compute == NULL) return LAZY_ERR_NULL_COMPUTE;
  if (out == NULL) return LAZY_ERR_NULL_OUTPUT;
  if (out_size == 0) return LAZY_ERR_INVALID_OUTPUT_SIZE;
  return LAZY_OK;
}

static LazyStatus validateLazyConfig(const Lazy *lazy) {
  if (lazy == NULL) return LAZY_ERR_NULL_OBJECT;
  if (lazy->state != LAZY_STATE_READY && lazy->state != LAZY_STATE_COMPUTED) {
    return LAZY_ERR_INVALID_STATE;
  }
  if (lazy->compute == NULL) return LAZY_ERR_NULL_COMPUTE;
  if (lazy->out == NULL) return LAZY_ERR_NULL_OUTPUT;
  if (lazy->out_size == 0) return LAZY_ERR_INVALID_OUTPUT_SIZE;
  return LAZY_OK;
}

LazyStatus lazy_create(Lazy **lazy, LazyComputeFn compute, void *ctx, void *out, size_t out_size) {
  LazyStatus status = validateLazyCreateArgs(lazy, compute, out, out_size);
  if (status != LAZY_OK) return status;

  *lazy = malloc(sizeof(**lazy));
  if (*lazy == NULL) return LAZY_ERR_ALLOC;

  (*lazy)->state = LAZY_STATE_READY;
  (*lazy)->compute = compute;
  (*lazy)->ctx = ctx;
  (*lazy)->out = out;
  (*lazy)->out_size = out_size;

  if (pthread_mutex_init(&(*lazy)->lock, NULL) != 0) {
    free(*lazy);
    *lazy = NULL;
    return LAZY_ERR_MUTEX_INIT;
  }

  return LAZY_OK;
}

LazyStatus lazy_eval(Lazy *lazy) {
  LazyStatus status = validateLazyConfig(lazy);
  if (status != LAZY_OK) return status;

  if (lazy->state == LAZY_STATE_COMPUTED) {
    return LAZY_OK;
  }

  if (pthread_mutex_lock(&lazy->lock) != 0) return LAZY_ERR_MUTEX_LOCK;

  if (lazy->state == LAZY_STATE_READY) {
    status = lazy->compute(lazy->ctx, lazy->out);
    if (status != LAZY_OK) {
      if (pthread_mutex_unlock(&lazy->lock) != 0) return LAZY_ERR_MUTEX_UNLOCK;
      return status;
    }
    lazy->state = LAZY_STATE_COMPUTED;
  }

  if (pthread_mutex_unlock(&lazy->lock) != 0) return LAZY_ERR_MUTEX_UNLOCK;
  return LAZY_OK;
}

LazyStatus lazy_reset(Lazy *lazy) {
  LazyStatus status = validateLazyConfig(lazy);
  if (status != LAZY_OK) return status;
  if (pthread_mutex_lock(&lazy->lock) != 0) return LAZY_ERR_MUTEX_LOCK;
  lazy->state = LAZY_STATE_READY;
  if (pthread_mutex_unlock(&lazy->lock) != 0) return LAZY_ERR_MUTEX_UNLOCK;
  return LAZY_OK;
}

LazyStatus lazy_is_computed(const Lazy *lazy, bool *computed) {
  LazyStatus status = validateLazyConfig(lazy);
  if (status != LAZY_OK) return status;
  if (computed == NULL) return LAZY_ERR_NULL_OUTPUT;
  *computed = lazy->state == LAZY_STATE_COMPUTED;
  return LAZY_OK;
}

LazyStatus lazy_destroy(Lazy *lazy) {
  if (lazy == NULL) return LAZY_ERR_NULL_OBJECT;
  if (lazy->state != LAZY_STATE_READY && lazy->state != LAZY_STATE_COMPUTED) {
    return LAZY_ERR_INVALID_STATE;
  }
  if (pthread_mutex_destroy(&lazy->lock) != 0) return LAZY_ERR_MUTEX_DESTROY;
  lazy->state = LAZY_STATE_DESTROYED;
  free(lazy);
  return LAZY_OK;
}
