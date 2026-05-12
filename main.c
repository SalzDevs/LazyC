#include <stdio.h>


typedef struct {
    void *data;
    int computed;

    void *result;
} MemoizeObject;


typedef struct {
    void (*code)(MemoizeObject *);  // now receives the memoObj
    MemoizeObject *memoObj;
    int callCount;
} LazyObject;


LazyObject lazyObjectInit(void (*code)(MemoizeObject *), MemoizeObject *memoObj) {
    return (LazyObject){ .code = code, .memoObj = memoObj, .callCount = 0 };
}

void executeLazyCode(LazyObject *lazyObj) {
    if (lazyObj->memoObj != NULL && lazyObj->memoObj->computed) {
        printf("Already computed, returning cached result.\n");
        return;
    }

    if (lazyObj->code != NULL) {
        lazyObj->code(lazyObj->memoObj);

        lazyObj->callCount++;
    }
}

void lazyCode(MemoizeObject *memoObj) {
    printf("Executing lazy code!\n");
    memoObj->result = "some result";  
    memoObj->computed = 1;           
}

int main() {
    MemoizeObject memoObj = { .data = NULL, .computed = 0, .result = NULL };
    LazyObject lazyObj = lazyObjectInit(lazyCode, &memoObj);

    executeLazyCode(&lazyObj);
    executeLazyCode(&lazyObj);

    printf("Lazy code executed %d time(s).\n", lazyObj.callCount);
    printf("Result: %s\n", (char *)memoObj.result);
    return 0;
}
