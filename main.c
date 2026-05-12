#include <stdio.h>


struct lazyObject {
    //points to code to be executed when the object is accessed
    void (*code)();
    // points to the data of the object
    void *data;
    // flag to check if the code has been executed (0 for not executed, 1 for executed)
    int executed; 
} lazyObject;

int main() {
    // initialize the lazy object
    lazyObject.code = NULL; 
    lazyObject.data = NULL;
    lazyObject.executed = 0;
    printf("Hello, World!\n");
    return 0;
}
