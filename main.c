#include <stdio.h>

#include "lazy.h"

LazyStatus IntensiveComputation(void *ctx, void *out) {
  (void)ctx;

  int *result = out;
  *result = 0;
  for (int i = 0; i < 100000000; i++) {
    *result += i;
  }
  return LAZY_OK;
}

int main() {
  int result = 0;
  bool computed = false;
  LazyObject *lazyObj = NULL;

  LazyStatus status = lazyObjectCreate(
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

  status = executeLazyCode(lazyObj);
  if (status != LAZY_OK) {
    printf("Eval failed\n");
    destroyLazyObj(lazyObj);
    return 0;
  }

  printf("Computed value: %d\n", result);

  status = lazyIsComputed(lazyObj, &computed);
  if (status != LAZY_OK) {
    printf("Computed check failed\n");
    destroyLazyObj(lazyObj);
    return 0;
  }
  printf("Computed: %s\n", computed ? "true" : "false");

  status = lazyReset(lazyObj);
  if (status != LAZY_OK) {
    printf("Reset failed\n");
    destroyLazyObj(lazyObj);
    return 0;
  }

  status = lazyIsComputed(lazyObj, &computed);
  if (status != LAZY_OK) {
    printf("Computed check failed\n");
    destroyLazyObj(lazyObj);
    return 0;
  }
  printf("Computed: %s\n", computed ? "true" : "false");

  status = destroyLazyObj(lazyObj);
  if (status != LAZY_OK) {
    printf("Destroy failed\n");
    return 0;
  }

  return 0;
}
