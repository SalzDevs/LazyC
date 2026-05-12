#include <stdio.h>
#include <time.h>

typedef struct {
    int computed;
    void *result;   
} MemoizeObject;

typedef struct {
    void (*code)(void *, MemoizeObject *);
    void *data;
    MemoizeObject *memoObj;
    int callCount;
    int useLazy;
} LazyObject;

LazyObject lazyObjectInit(void (*code)(void *, MemoizeObject *), void *data, MemoizeObject *memoObj, int useLazy) {
    return (LazyObject){ .code = code, .data = data, .memoObj = memoObj, .callCount = 0, .useLazy = useLazy };
}


void executeLazyCode(LazyObject *lazyObj) {
    if (lazyObj->memoObj == NULL) return;
    if (lazyObj->useLazy && lazyObj->memoObj->computed) {
        printf("Cached result: %ld\n", *(long *)lazyObj->memoObj->result);
        return;
    }
    if (lazyObj->code != NULL) {
        lazyObj->code(lazyObj->data, lazyObj->memoObj);
        lazyObj->callCount++;
    }
}

void resetComputed(LazyObject *lazyObj) {
    if (lazyObj->memoObj == NULL) return;
    lazyObj->memoObj->computed = 0;
}

void lazyCode(void *data, MemoizeObject *memoObj) {
    long iterations = *((long *)data);
    printf("Computing for %ld iterations...\n", iterations);


    volatile long result = 0;
    for (long i = 0; i < iterations; i++) result += i;


    static long stored;     
    stored = result;
    memoObj->result = &stored;

    memoObj->computed = 1;
    printf("Done. Result: %ld\n", *(long *)memoObj->result);
}

int main(int argc, char *argv[]) {
    int useLazy = 1;
    if (argc > 1 && argv[1][0] == '0') useLazy = 0;


    printf("Mode: %s\n\n", useLazy ? "LAZY" : "EAGER");


    long iterations = 1000000000L;
    MemoizeObject memoObj = { .computed = 0, .result = NULL };
    LazyObject lazyObj = lazyObjectInit(lazyCode, &iterations, &memoObj, useLazy);

    clock_t start = clock();

    executeLazyCode(&lazyObj);
    executeLazyCode(&lazyObj);
    executeLazyCode(&lazyObj);

    printf("\nTotal executions: %d\n", lazyObj.callCount);
    printf("Final result: %ld\n", *(long *)memoObj.result);
    printf("Time elapsed: %.2fs\n", (double)(clock() - start) / CLOCKS_PER_SEC);


    return 0;
}
