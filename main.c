#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>

typedef void (*LazyComputeFn)(void *ctx, void *out);

typedef enum {
  LAZY_OK = 0,
  LAZY_ERR_NULL_OBJECT,
  LAZY_ERR_NULL_COMPUTE,
  LAZY_ERR_NULL_OUTPUT,
  LAZY_ERR_INVALID_OUTPUT_SIZE,
  LAZY_ERR_MUTEX_INIT,
} LazyStatus;

typedef struct {
  LazyComputeFn compute;
  bool computed;
  void *ctx;
  void *out;
  size_t outSize;
  pthread_mutex_t lock;
} LazyObject;

LazyStatus validateLazyObjectConfig(const LazyObject *lazyObj, LazyComputeFn compute, void *out, size_t outSize) {
  if (lazyObj == NULL) return LAZY_ERR_NULL_OBJECT;
  if (compute == NULL) return LAZY_ERR_NULL_COMPUTE;
  if (out == NULL) return LAZY_ERR_NULL_OUTPUT;
  if (outSize == 0) return LAZY_ERR_INVALID_OUTPUT_SIZE;
  return LAZY_OK;
}

LazyStatus lazyObjectInit(LazyObject *lazyObj, LazyComputeFn compute, void *ctx, void *out, size_t outSize) {
  LazyStatus status = validateLazyObjectConfig(lazyObj, compute, out, outSize);
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

void executeLazyCode(LazyObject *lazyObj) {
  //TODO: Handle this as an error 
  if (lazyObj == NULL) return;

  if (lazyObj->computed) {
    return;
  }

  pthread_mutex_lock(&lazyObj->lock);
  
  if (!lazyObj->computed) {
    lazyObj->compute(lazyObj->ctx, lazyObj->out);
    lazyObj->computed = true;
  }

  pthread_mutex_unlock(&lazyObj->lock);
}


void resetComputed(LazyObject *lazyObj) {
  if (lazyObj == NULL) return;
  pthread_mutex_lock(&lazyObj->lock);
  lazyObj->computed = false;
  pthread_mutex_unlock(&lazyObj->lock); 
}

void IntensiveComputation(void *ctx, void *out) {
  int *result = out;
  *result = 0;
  for (int i = 0; i < 100000000; i++) {
    *result += i;
  }
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
      0
  );
  printf("status: %d\n", status);
  if (status != LAZY_OK) {
    printf("INVALID CONFIG\n");
    return 0;
  }

  executeLazyCode(&lazyObj);
  printf("Computed value: %d\n", result);
  printf("Computed: ");
  printf(lazyObj.computed ? "true\n" : "false\n");
  resetComputed(&lazyObj);
  printf("Computed: ");
  printf(lazyObj.computed ? "true\n" : "false\n");
  return 0;
}
