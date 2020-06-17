#include <math.h>
#include <unistd.h>

void* smalloc(size_t size){
    if(size <= 0 || size > size_t(pow(10.0,8.0))){
        return nullptr;
    }
    void* ptr = sbrk(size);
    return ptr == (void*)(-1) ? nullptr : ptr;
}
