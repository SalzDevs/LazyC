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

LazyObject lazyObjectInit(LazyComputeFn compute, void *ctx, size_t outSize) {
  LazyObject lazyObj = {
    .compute = compute,
    .ctx = ctx,
    .outSize = outSize,
    .out = NULL
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
  
  if (lazyObj->out == NULL) { // Double-checked locking
    lazyObj->compute(lazyObj->ctx, &lazyObj->out);
    lazyObj->computed = 1;
  }

  pthread_mutex_unlock(&lazyObj->lock);
}


void resetComputed(LazyObject *lazyObj) {
  if (lazyObj == NULL) return;
  pthread_mutex_lock(&lazyObj->lock);
  lazyObj->computed = 0;
  lazyObj->out = NULL;
  pthread_mutex_unlock(&lazyObj->lock); 
}

void IntensiveComputation(void *ctx, void *out) {
  int *result = (int *)malloc(sizeof(int));
  *result = 42;
  *(int **)out = result;
}

void destroyLazyObj(LazyObject *lazyObj) {
  if (lazyObj == NULL) return;
  pthread_mutex_destroy(&lazyObj->lock); 
}

int main() {
  //Demo code 
  LazyObject lazyObj = lazyObjectInit(
      IntensiveComputation,
      NULL,
      sizeof(int *)
  );
  executeLazyCode(&lazyObj);
  printf("Computed value: %d\n", *(int *)lazyObj.out);
  resetComputed(&lazyObj);
  return 0;
}
