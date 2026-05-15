#include "lazy.h"

#include <pthread.h>
#include <stdlib.h>

typedef enum {
  LAZY_STATE_UNINITIALIZED = 0,
  LAZY_STATE_READY,
  LAZY_STATE_COMPUTED,
  LAZY_STATE_DESTROYED
} LazyState;

struct LazyObject {
  LazyComputeFn compute;
  LazyState state;
  void *ctx;
  void *out;
  size_t outSize;
  pthread_mutex_t lock;
};

static LazyStatus validateLazyObjectCreateArgs(LazyObject **lazyObj, LazyComputeFn compute, void *out, size_t outSize) {
  if (lazyObj == NULL) return LAZY_ERR_NULL_OBJECT;
  if (compute == NULL) return LAZY_ERR_NULL_COMPUTE;
  if (out == NULL) return LAZY_ERR_NULL_OUTPUT;
  if (outSize == 0) return LAZY_ERR_INVALID_OUTPUT_SIZE;
  return LAZY_OK;
}

static LazyStatus validateLazyObjectConfig(const LazyObject *lazyObj) {
  if (lazyObj == NULL) return LAZY_ERR_NULL_OBJECT;
  if (lazyObj->state != LAZY_STATE_READY && lazyObj->state != LAZY_STATE_COMPUTED) {
    return LAZY_ERR_INVALID_STATE;
  }
  if (lazyObj->compute == NULL) return LAZY_ERR_NULL_COMPUTE;
  if (lazyObj->out == NULL) return LAZY_ERR_NULL_OUTPUT;
  if (lazyObj->outSize == 0) return LAZY_ERR_INVALID_OUTPUT_SIZE;
  return LAZY_OK;
}

LazyStatus lazyObjectCreate(LazyObject **lazyObj, LazyComputeFn compute, void *ctx, void *out, size_t outSize) {
  LazyStatus status = validateLazyObjectCreateArgs(lazyObj, compute, out, outSize);
  if (status != LAZY_OK) return status;

  *lazyObj = malloc(sizeof(**lazyObj));
  if (*lazyObj == NULL) return LAZY_ERR_ALLOC;

  (*lazyObj)->state = LAZY_STATE_READY;
  (*lazyObj)->compute = compute;
  (*lazyObj)->ctx = ctx;
  (*lazyObj)->out = out;
  (*lazyObj)->outSize = outSize;

  if (pthread_mutex_init(&(*lazyObj)->lock, NULL) != 0) {
    free(*lazyObj);
    *lazyObj = NULL;
    return LAZY_ERR_MUTEX_INIT;
  }

  return LAZY_OK;
}

LazyStatus executeLazyCode(LazyObject *lazyObj) {
  LazyStatus status = validateLazyObjectConfig(lazyObj);
  if (status != LAZY_OK) return status;

  if (lazyObj->state == LAZY_STATE_COMPUTED) {
    return LAZY_OK;
  }

  if (pthread_mutex_lock(&lazyObj->lock) != 0) return LAZY_ERR_MUTEX_LOCK;

  if (lazyObj->state == LAZY_STATE_READY) {
    status = lazyObj->compute(lazyObj->ctx, lazyObj->out);
    if (status != LAZY_OK) {
      if (pthread_mutex_unlock(&lazyObj->lock) != 0) return LAZY_ERR_MUTEX_UNLOCK;
      return status;
    }
    lazyObj->state = LAZY_STATE_COMPUTED;
  }

  if (pthread_mutex_unlock(&lazyObj->lock) != 0) return LAZY_ERR_MUTEX_UNLOCK;
  return LAZY_OK;
}

LazyStatus lazyReset(LazyObject *lazyObj) {
  LazyStatus status = validateLazyObjectConfig(lazyObj);
  if (status != LAZY_OK) return status;
  if (pthread_mutex_lock(&lazyObj->lock) != 0) return LAZY_ERR_MUTEX_LOCK;
  lazyObj->state = LAZY_STATE_READY;
  if (pthread_mutex_unlock(&lazyObj->lock) != 0) return LAZY_ERR_MUTEX_UNLOCK;
  return LAZY_OK;
}

LazyStatus lazyIsComputed(const LazyObject *lazyObj, bool *computed) {
  LazyStatus status = validateLazyObjectConfig(lazyObj);
  if (status != LAZY_OK) return status;
  if (computed == NULL) return LAZY_ERR_NULL_OUTPUT;
  *computed = lazyObj->state == LAZY_STATE_COMPUTED;
  return LAZY_OK;
}

LazyStatus destroyLazyObj(LazyObject *lazyObj) {
  if (lazyObj == NULL) return LAZY_ERR_NULL_OBJECT;
  if (lazyObj->state != LAZY_STATE_READY && lazyObj->state != LAZY_STATE_COMPUTED) {
    return LAZY_ERR_INVALID_STATE;
  }
  if (pthread_mutex_destroy(&lazyObj->lock) != 0) return LAZY_ERR_MUTEX_DESTROY;
  lazyObj->state = LAZY_STATE_DESTROYED;
  free(lazyObj);
  return LAZY_OK;
}
