#include <stdio.h>

typedef struct {
    void (*code)();  // executed on first access
    void *data;
    int executed;    // 0 = pending, 1 = done
} LazyObject;

LazyObject lazyObjectInit(void (*code)(), void *data) {

    return (LazyObject){ .code = code, .data = data, .executed = 0 };
}

int main() {
    LazyObject lazyObj = lazyObjectInit(NULL, NULL);
    printf("Hello, World123!\n");
    return 0;
}
