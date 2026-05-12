#include <stdio.h>

#include <time.h>
#include <pthread.h>

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
    pthread_mutex_t lock;
} LazyObject;

LazyObject lazyObjectInit(void (*code)(void *, MemoizeObject *), void *data, MemoizeObject *memoObj, int useLazy) {
    LazyObject lazyObj = { .code = code, .data = data, .memoObj = memoObj, .callCount = 0, .useLazy = useLazy };
    pthread_mutex_init(&lazyObj.lock, NULL);
    return lazyObj; 
}

void executeLazyCode(LazyObject *lazyObj) {
    if (lazyObj->memoObj == NULL) return;
    pthread_mutex_lock(&lazyObj->lock);
    if (lazyObj->useLazy && lazyObj->memoObj->computed) {
        printf("  [hit]  cached result: %ld\n", *(long *)lazyObj->memoObj->result);
    } else if (lazyObj->code != NULL) {
        lazyObj->code(lazyObj->data, lazyObj->memoObj);
        lazyObj->callCount++;
    }
    pthread_mutex_unlock(&lazyObj->lock);

}


void resetComputed(LazyObject *lazyObj) {
    if (lazyObj->memoObj == NULL) return;
    pthread_mutex_lock(&lazyObj->lock);
    lazyObj->memoObj->computed = 0;
    pthread_mutex_unlock(&lazyObj->lock);
}

void lazyCode(void *data, MemoizeObject *memoObj) {
    long iterations = *((long *)data);
    printf("  [miss] computing %ld iterations...\n", iterations);
    volatile long result = 0;
    for (long i = 0; i < iterations; i++) result += i;
    static long stored;
    stored = result;
    memoObj->result = &stored;
    memoObj->computed = 1;
    printf("  [done] result: %ld\n", *(long *)memoObj->result);

}


void destroyLazyObj(LazyObject *lazyObj) {
    if (lazyObj) {
        pthread_mutex_destroy(&lazyObj->lock);

    }
}

int main(int argc, char *argv[]) {
    int useLazy = 1;
    if (argc > 1 && argv[1][0] == '0') useLazy = 0;

    printf("┌─────────────────────────────────────┐\n");
    printf("│  lazy eval in C  │  mode: %-8s  │\n", useLazy ? "LAZY" : "EAGER");
    printf("└─────────────────────────────────────┘\n\n");

    long iterations = 1000000000L;
    MemoizeObject memoObj = { .computed = 0, .result = NULL };
    LazyObject lazyObj = lazyObjectInit(lazyCode, &iterations, &memoObj, useLazy);

    clock_t start = clock();


    printf("call 1:\n"); executeLazyCode(&lazyObj);
    printf("call 2:\n"); executeLazyCode(&lazyObj);
    printf("call 3:\n"); executeLazyCode(&lazyObj);

    double elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;

    printf("┌─────────────────────────────────────┐\n");
    printf("│  actual executions : %-3d            │\n", lazyObj.callCount);
    printf("│  total calls       : 3              │\n");
    printf("│  time elapsed      : %-6.2fs        │\n", elapsed);
    printf("└─────────────────────────────────────┘\n");

    destroyLazyObj(&lazyObj);
    return 0;
}
