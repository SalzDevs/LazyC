#include <stdio.h>
#include <time.h>

typedef struct {
    void *data;
    int computed;
    void *result;
} MemoizeObject;

typedef struct {
    void (*code)(MemoizeObject *);
    MemoizeObject *memoObj;
    int callCount;
    int useLazy;
} LazyObject;

LazyObject lazyObjectInit(void (*code)(MemoizeObject *), MemoizeObject *memoObj, int useLazy) {
    return (LazyObject){ .code = code, .memoObj = memoObj, .callCount = 0, .useLazy = useLazy };
}

void executeLazyCode(LazyObject *lazyObj) {
    if (lazyObj->useLazy && lazyObj->memoObj != NULL && lazyObj->memoObj->computed) {

        printf("Already computed, returning cached result.\n");
        return;
    }
    if (lazyObj->code != NULL) {
        lazyObj->code(lazyObj->memoObj);
        lazyObj->callCount++;
    }
}

void lazyCode(MemoizeObject *memoObj) {
    printf("Starting heavy computation...\n");

    volatile long result = 0;
    for (long i = 0; i < 1000000000L; i++) {
        result += i;
    }

    printf("Done. Result: %ld\n", result);
    memoObj->result = "computation complete";
    memoObj->computed = 1;
}

int main(int argc, char *argv[]) {
    int useLazy = 1;
    if (argc > 1 && argv[1][0] == '0') useLazy = 0;

    printf("Mode: %s\n\n", useLazy ? "LAZY (memoized)" : "EAGER (no cache)");

    MemoizeObject memoObj = { .data = NULL, .computed = 0, .result = NULL };
    LazyObject lazyObj = lazyObjectInit(lazyCode, &memoObj, useLazy);

    clock_t start = clock();

    executeLazyCode(&lazyObj);
    executeLazyCode(&lazyObj);
    executeLazyCode(&lazyObj);
    executeLazyCode(&lazyObj);
    executeLazyCode(&lazyObj);
    executeLazyCode(&lazyObj);
    executeLazyCode(&lazyObj);
    executeLazyCode(&lazyObj);
    executeLazyCode(&lazyObj);

    printf("\nTotal executions: %d\n", lazyObj.callCount);
    printf("Time elapsed: %.2fs\n", (double)(clock() - start) / CLOCKS_PER_SEC);

    return 0;
}
