#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>

typedef void (*LazyComputeFn)(void *ctx, void *out);

typedef struct {
  LazyComputeFn compute;
  int computed;
  void *ctx;
  void *out;
  size_t outSize;
  pthread_mutex_t lock;
} LazyObject;

LazyObject lazyObjectInit(LazyComputeFn compute, void *ctx, void *out, size_t outSize) {
  LazyObject lazyObj = {
    .compute = compute,
    .ctx = ctx,
    .out = out,
    .outSize = outSize,
    .computed = 0
  };

  pthread_mutex_init(&lazyObj.lock, NULL);
  return lazyObj;
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
    lazyObj->computed = 1;
  }

  pthread_mutex_unlock(&lazyObj->lock);
}


void resetComputed(LazyObject *lazyObj) {
  if (lazyObj == NULL) return;
  pthread_mutex_lock(&lazyObj->lock);
  lazyObj->computed = 0;
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
  LazyObject lazyObj = lazyObjectInit(
      IntensiveComputation,
      NULL,
      &result,
      sizeof(result)
  );
  executeLazyCode(&lazyObj);
  printf("Computed value: %d\n", result);
  printf("Computed? %d\n", lazyObj.computed);
  resetComputed(&lazyObj);
  printf("Computed after reset? %d\n", lazyObj.computed);
  return 0;
}
