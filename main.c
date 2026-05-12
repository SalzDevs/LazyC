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
    LazyObject lazyObj =  { .code = code, .data = data, .memoObj = memoObj, .callCount = 0, .useLazy = useLazy };
    pthread_mutex_init(&lazyObj.lock,NULL);
    return lazyObj; 
}


void executeLazyCode(LazyObject *lazyObj) {
    if (lazyObj->memoObj == NULL) return;
    
    pthread_mutex_lock(&lazyObj->lock);

    if (lazyObj->useLazy && lazyObj->memoObj->computed) {
        //TODO: this should be agnostic (we should have a way to if we want to print cast were without knowing the type)
        printf("Cached result: %ld\n", *(long *)lazyObj->memoObj->result);
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
    printf("Computing for %ld iterations...\n", iterations);


    volatile long result = 0;
    for (long i = 0; i < iterations; i++) result += i;


    static long stored;     
    stored = result;
    memoObj->result = &stored;

    memoObj->computed = 1;
    printf("Done. Result: %ld\n", *(long *)memoObj->result);
}

void destroyLazyObj(LazyObject *lazyObj) {
    if (lazyObj) {
        pthread_mutex_destroy(&lazyObj->lock);
        printf("Lazy Object destroyed...\n");
    }
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

    destroyLazyObj(&lazyObj);
    return 0;
}
