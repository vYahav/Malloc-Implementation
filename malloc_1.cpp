#include <math.h>
#include <unistd.h>

void* smalloc(size_t size){
    if(size <= 0 || size > pow(10,8)){
        return nullptr;
    }
    void* ptr = sbrk(size);
    return ptr == (void*)(-1) ? nullptr : ptr;
}
