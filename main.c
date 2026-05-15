#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>

typedef enum {
  LAZY_OK = 0,
  LAZY_ERR_NULL_OBJECT,
  LAZY_ERR_NULL_COMPUTE,
  LAZY_ERR_NULL_OUTPUT,
  LAZY_ERR_INVALID_OUTPUT_SIZE,
  LAZY_ERR_MUTEX_INIT,
  LAZY_ERR_COMPUTE,
} LazyStatus;

typedef LazyStatus (*LazyComputeFn)(void *ctx, void *out);

typedef struct {
  LazyComputeFn compute;
  bool computed;
  void *ctx;
  void *out;
  size_t outSize;
  pthread_mutex_t lock;
} LazyObject;

LazyStatus validateLazyObjectInitArgs(const LazyObject *lazyObj, LazyComputeFn compute, void *out, size_t outSize) {
  if (lazyObj == NULL) return LAZY_ERR_NULL_OBJECT;
  if (compute == NULL) return LAZY_ERR_NULL_COMPUTE;
  if (out == NULL) return LAZY_ERR_NULL_OUTPUT;
  if (outSize == 0) return LAZY_ERR_INVALID_OUTPUT_SIZE;
  return LAZY_OK;
}

LazyStatus validateLazyObjectConfig(const LazyObject *lazyObj) {
  if (lazyObj == NULL) return LAZY_ERR_NULL_OBJECT;
  return validateLazyObjectInitArgs(lazyObj, lazyObj->compute, lazyObj->out, lazyObj->outSize);
}

LazyStatus lazyObjectInit(LazyObject *lazyObj, LazyComputeFn compute, void *ctx, void *out, size_t outSize) {
  LazyStatus status = validateLazyObjectInitArgs(lazyObj, compute, out, outSize);
  if (status != LAZY_OK) return status;

  lazyObj->computed = false;
  lazyObj->compute = compute;
  lazyObj->ctx = ctx;
  lazyObj->out = out;
  lazyObj->outSize = outSize;

  if (pthread_mutex_init(&lazyObj->lock, NULL) != 0) {
    return LAZY_ERR_MUTEX_INIT;
  }

  return LAZY_OK;
}

LazyStatus executeLazyCode(LazyObject *lazyObj) {
  LazyStatus status = validateLazyObjectConfig(lazyObj);
  if (status!=LAZY_OK) return status;
  
  if (lazyObj->computed) {
    return LAZY_OK;
  }

  pthread_mutex_lock(&lazyObj->lock);
  
  if (!lazyObj->computed) {
    status = lazyObj->compute(lazyObj->ctx, lazyObj->out);
    if (status != LAZY_OK) {
      pthread_mutex_unlock(&lazyObj->lock);
      return status;
    }
    lazyObj->computed = true;
  }

  pthread_mutex_unlock(&lazyObj->lock);
  return LAZY_OK;
}


LazyStatus lazyReset(LazyObject *lazyObj) {
  LazyStatus status = validateLazyObjectConfig(lazyObj); 
  if (status != LAZY_OK) return status;
  pthread_mutex_lock(&lazyObj->lock);
  lazyObj->computed = false;
  pthread_mutex_unlock(&lazyObj->lock);
  return LAZY_OK;
}

LazyStatus IntensiveComputation(void *ctx, void *out) {
  int *result = out;
  *result = 0;
  for (int i = 0; i < 100000000; i++) {
    *result += i;
  }
  return LAZY_OK;
}

void destroyLazyObj(LazyObject *lazyObj) {
  if (lazyObj == NULL) return;
  pthread_mutex_destroy(&lazyObj->lock); 
}

int main() {
  int result = 0;
  LazyObject lazyObj;
  LazyStatus status = lazyObjectInit(
      &lazyObj,
      IntensiveComputation,
      NULL,
      &result,
      sizeof(result) 
  );
  printf("status: %d\n", status);
  if (status != LAZY_OK) {
    printf("INVALID CONFIG\n");
    return 0;
  }

  status = executeLazyCode(&lazyObj);

  if (status != LAZY_OK) {
    printf("Invalid Config\n");
    return 0;
  }

  printf("Computed value: %d\n", result);
  printf("Computed: ");
  printf(lazyObj.computed ? "true\n" : "false\n");
  status = lazyReset(&lazyObj);
  if (status != LAZY_OK) {
    printf("Invalid Config\n");
    return 0;
  }
  printf("Computed: ");
  printf(lazyObj.computed ? "true\n" : "false\n");
  destroyLazyObj(&lazyObj);
  return 0;
}
