# lazyC

A small lazy-evaluation helper for C.

`lazyC` lets you wrap a computation and run it only once. Later evaluations reuse
the cached state until you reset it.

## Example

```c
#include "lazy.h"

LazyStatus compute_answer(void *ctx, void *out) {
  (void)ctx;
  *(int *)out = 42;
  return LAZY_OK;
}

int main(void) {
  int result = 0;
  Lazy *lazy = NULL;

  if (lazy_create(&lazy, compute_answer, NULL, &result, sizeof(result)) != LAZY_OK) {
    return 1;
  }

  lazy_eval(lazy); // computes result
  lazy_eval(lazy); // cache hit; compute_answer is not called again

  lazy_destroy(lazy);
  return 0;
}
```

## Ownership model

The caller owns the result storage.

Pass a pointer to output storage into `lazy_create()`:

```c
int result = 0;
lazy_create(&lazy, compute, ctx, &result, sizeof(result));
```

The library does not allocate, copy, clear, or free the result value. The output
storage must remain valid until `lazy_destroy()` returns.

`ctx` may be `NULL`.

## Reset behavior

`lazy_reset()` only marks the lazy value as not computed. It does not clear the
caller-owned output storage. After reset, the output may contain stale data until
the next successful `lazy_eval()`.

## Thread safety

`lazy_eval()`, `lazy_reset()`, and `lazy_is_computed()` are synchronized for use
from multiple threads on the same `Lazy` object.

`lazy_destroy()` must not run concurrently with any other operation on the same
`Lazy` object. The caller is responsible for ensuring no other thread is using
the object before destroying it.

## Build and test

```sh
make
make test
make sanitize
```

`make sanitize` builds and runs the test harness with AddressSanitizer and UBSan.

## Public API

```c
typedef struct Lazy Lazy;
typedef LazyStatus (*LazyComputeFn)(void *ctx, void *out);

LazyStatus lazy_create(Lazy **lazy, LazyComputeFn compute, void *ctx, void *out, size_t out_size);
LazyStatus lazy_eval(Lazy *lazy);
LazyStatus lazy_reset(Lazy *lazy);
LazyStatus lazy_destroy(Lazy *lazy);
LazyStatus lazy_is_computed(Lazy *lazy, bool *computed);
```

## Current limitations

- Requires pthreads.
- `lazy_destroy()` is not safe to call concurrently with other operations.
- The library stores one caller-owned output pointer per lazy object.
- No packaged install target yet.
