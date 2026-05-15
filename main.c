#include <stdio.h>
#include "lazy.h"

LazyStatus compute_answer(void *ctx, void *out) {
  int *calls = ctx;
  (*calls)++;
  puts("cache miss: computing...");
  *(int *)out = 42;
  return LAZY_OK;
}

int main(void) {
  int answer = 0, calls = 0;
  Lazy *lazy = NULL;
  lazy_create(&lazy, compute_answer, &calls, &answer, sizeof(answer));

  lazy_eval(lazy); printf("answer=%d calls=%d\n", answer, calls);
  lazy_eval(lazy); printf("answer=%d calls=%d (cached)\n", answer, calls);
  lazy_reset(lazy);
  lazy_eval(lazy); printf("answer=%d calls=%d (recomputed)\n", answer, calls);

  lazy_destroy(lazy);
  return 0;
}
