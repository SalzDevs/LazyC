#include <stdbool.h>
#include <stdio.h>

#include "../lazy.h"

#define ASSERT_TRUE(expr) \
  do { \
    if (!(expr)) { \
      printf("FAIL: %s:%d: %s\n", __FILE__, __LINE__, #expr); \
      return 1; \
    } \
  } while (0)

#define ASSERT_EQ_INT(expected, actual) \
  do { \
    int expected_value = (expected); \
    int actual_value = (actual); \
    if (expected_value != actual_value) { \
      printf("FAIL: %s:%d: expected %d, got %d\n", __FILE__, __LINE__, expected_value, actual_value); \
      return 1; \
    } \
  } while (0)

typedef struct {
  int calls;
  int value;
  LazyStatus status;
} TestCtx;

static LazyStatus compute_value(void *ctx, void *out) {
  TestCtx *test_ctx = ctx;
  test_ctx->calls++;

  if (test_ctx->status != LAZY_OK) {
    return test_ctx->status;
  }

  *(int *)out = test_ctx->value;
  return LAZY_OK;
}

static int test_create_rejects_invalid_args(void) {
  Lazy *lazy = (Lazy *)0x1;
  int out = 0;
  TestCtx ctx = {0};

  ASSERT_EQ_INT(LAZY_ERR_NULL_OBJECT, lazy_create(NULL, compute_value, &ctx, &out, sizeof(out)));

  ASSERT_EQ_INT(LAZY_ERR_NULL_COMPUTE, lazy_create(&lazy, NULL, &ctx, &out, sizeof(out)));
  ASSERT_TRUE(lazy == NULL);

  lazy = (Lazy *)0x1;
  ASSERT_EQ_INT(LAZY_ERR_NULL_OUTPUT, lazy_create(&lazy, compute_value, &ctx, NULL, sizeof(out)));
  ASSERT_TRUE(lazy == NULL);

  lazy = (Lazy *)0x1;
  ASSERT_EQ_INT(LAZY_ERR_INVALID_OUTPUT_SIZE, lazy_create(&lazy, compute_value, &ctx, &out, 0));
  ASSERT_TRUE(lazy == NULL);

  return 0;
}

static int test_eval_computes_once(void) {
  Lazy *lazy = NULL;
  int out = 0;
  bool computed = true;
  TestCtx ctx = {.calls = 0, .value = 42, .status = LAZY_OK};

  ASSERT_EQ_INT(LAZY_OK, lazy_create(&lazy, compute_value, &ctx, &out, sizeof(out)));
  ASSERT_EQ_INT(LAZY_OK, lazy_is_computed(lazy, &computed));
  ASSERT_TRUE(!computed);

  ASSERT_EQ_INT(LAZY_OK, lazy_eval(lazy));
  ASSERT_EQ_INT(42, out);
  ASSERT_EQ_INT(1, ctx.calls);
  ASSERT_EQ_INT(LAZY_OK, lazy_is_computed(lazy, &computed));
  ASSERT_TRUE(computed);

  ASSERT_EQ_INT(LAZY_OK, lazy_eval(lazy));
  ASSERT_EQ_INT(1, ctx.calls);

  ASSERT_EQ_INT(LAZY_OK, lazy_destroy(lazy));
  return 0;
}

static int test_reset_allows_recompute(void) {
  Lazy *lazy = NULL;
  int out = 0;
  bool computed = true;
  TestCtx ctx = {.calls = 0, .value = 7, .status = LAZY_OK};

  ASSERT_EQ_INT(LAZY_OK, lazy_create(&lazy, compute_value, &ctx, &out, sizeof(out)));
  ASSERT_EQ_INT(LAZY_OK, lazy_eval(lazy));
  ASSERT_EQ_INT(1, ctx.calls);

  ASSERT_EQ_INT(LAZY_OK, lazy_reset(lazy));
  ASSERT_EQ_INT(LAZY_OK, lazy_is_computed(lazy, &computed));
  ASSERT_TRUE(!computed);

  ctx.value = 9;
  ASSERT_EQ_INT(LAZY_OK, lazy_eval(lazy));
  ASSERT_EQ_INT(9, out);
  ASSERT_EQ_INT(2, ctx.calls);

  ASSERT_EQ_INT(LAZY_OK, lazy_destroy(lazy));
  return 0;
}

static int test_compute_failure_does_not_mark_computed(void) {
  Lazy *lazy = NULL;
  int out = 0;
  bool computed = true;
  TestCtx ctx = {.calls = 0, .value = 42, .status = LAZY_ERR_COMPUTE};

  ASSERT_EQ_INT(LAZY_OK, lazy_create(&lazy, compute_value, &ctx, &out, sizeof(out)));
  ASSERT_EQ_INT(LAZY_ERR_COMPUTE, lazy_eval(lazy));
  ASSERT_EQ_INT(1, ctx.calls);
  ASSERT_EQ_INT(LAZY_OK, lazy_is_computed(lazy, &computed));
  ASSERT_TRUE(!computed);

  ctx.status = LAZY_OK;
  ASSERT_EQ_INT(LAZY_OK, lazy_eval(lazy));
  ASSERT_EQ_INT(2, ctx.calls);
  ASSERT_EQ_INT(42, out);
  ASSERT_EQ_INT(LAZY_OK, lazy_is_computed(lazy, &computed));
  ASSERT_TRUE(computed);

  ASSERT_EQ_INT(LAZY_OK, lazy_destroy(lazy));
  return 0;
}

static int test_null_operations(void) {
  bool computed = false;

  ASSERT_EQ_INT(LAZY_ERR_NULL_OBJECT, lazy_eval(NULL));
  ASSERT_EQ_INT(LAZY_ERR_NULL_OBJECT, lazy_reset(NULL));
  ASSERT_EQ_INT(LAZY_ERR_NULL_OBJECT, lazy_destroy(NULL));
  ASSERT_EQ_INT(LAZY_ERR_NULL_OBJECT, lazy_is_computed(NULL, &computed));

  return 0;
}

int main(void) {
  ASSERT_EQ_INT(0, test_create_rejects_invalid_args());
  ASSERT_EQ_INT(0, test_eval_computes_once());
  ASSERT_EQ_INT(0, test_reset_allows_recompute());
  ASSERT_EQ_INT(0, test_compute_failure_does_not_mark_computed());
  ASSERT_EQ_INT(0, test_null_operations());

  printf("All lazy tests passed\n");
  return 0;
}
