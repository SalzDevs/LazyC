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
  Lazy *lazy = NULL;

  LazyStatus status = lazy_create(
      &lazy,
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

  status = lazy_eval(lazy);
  if (status != LAZY_OK) {
    printf("Eval failed\n");
    lazy_destroy(lazy);
    return 0;
  }

  printf("Computed value: %d\n", result);

  status = lazy_is_computed(lazy, &computed);
  if (status != LAZY_OK) {
    printf("Computed check failed\n");
    lazy_destroy(lazy);
    return 0;
  }
  printf("Computed: %s\n", computed ? "true" : "false");

  status = lazy_reset(lazy);
  if (status != LAZY_OK) {
    printf("Reset failed\n");
    lazy_destroy(lazy);
    return 0;
  }

  status = lazy_is_computed(lazy, &computed);
  if (status != LAZY_OK) {
    printf("Computed check failed\n");
    lazy_destroy(lazy);
    return 0;
  }
  printf("Computed: %s\n", computed ? "true" : "false");

  status = lazy_destroy(lazy);
  if (status != LAZY_OK) {
    printf("Destroy failed\n");
    return 0;
  }

  return 0;
}
