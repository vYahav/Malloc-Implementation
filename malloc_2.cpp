#include <math.h>
#include <cstring>
#include <unistd.h>

using namespace std;

typedef struct metadata_t {
    size_t size;
    bool is_free;
    metadata_t* next;
} *metadata;

metadata first = nullptr;
metadata last = nullptr;

//TODO: check if sizeof(ptr + struct metadata_t) OR sizeof(ptr + 1)

void* smalloc(size_t size){
    if(size <= 0 || size > (size_t)(pow(10.0,8.0))){
        return nullptr;
    }
    if (first == nullptr || (first->size >= size && first->is_free)){
        //TODO: check if allocation really puts metadata first!!
        metadata alloc_ptr = (metadata)sbrk(0);
        void* new_block = sbrk(size + sizeof(struct metadata_t));
        if(new_block == (void*)(-1)){
            return nullptr;
        }
        if(last == nullptr){
            last = alloc_ptr; //the program break after sbrk
        }
        if(first == nullptr){
            alloc_ptr->next = nullptr;
            first = alloc_ptr; //the program break before sbrk
        }
        alloc_ptr->size = size;
        alloc_ptr->is_free = false;
        return (void*)(alloc_ptr + sizeof(struct metadata_t));
    }
    metadata it = first;
    while(it->next != nullptr){
        if(it->next->is_free && it->next->size >= size){
            break;
        }
        it = it->next;
    }
    if(it->next == nullptr){
        //meaning we reached end of our allocation list
        metadata alloc_ptr = (metadata)sbrk(0);
        void* new_block = sbrk(size + sizeof(struct metadata_t));
        if(new_block == (void*)(-1)){
            return nullptr;
        }
        last = alloc_ptr;
        it->next = alloc_ptr;
        alloc_ptr->size = size;
        alloc_ptr->is_free = false;
        alloc_ptr->next = nullptr;
        return (void*)(alloc_ptr + sizeof(struct metadata_t));
    }
    it->next->is_free = false;
    it->next->size = size;
    return(it->next + sizeof(struct metadata_t));
}

void* scalloc (size_t num, size_t size){
    void* ptr = smalloc(size * num);
    if(nullptr == ptr){
        return nullptr;
    }
    memset(ptr, 0, size * num);
    return ptr;
}

void sfree(void* p){
    if(nullptr == p){
        return;
    }
    metadata met = (metadata)p - sizeof(struct metadata_t);
    met->is_free = true;
}

void* srealloc(void* oldp, size_t size){
    if(size <= 0 || size > (size_t)(pow(10.0, 8.0))){
        return nullptr;
    }
    if(nullptr == oldp){
        return smalloc(size);
    }
    metadata old_ptr = (metadata)oldp - sizeof(struct metadata_t);
    if(old_ptr->size >= size){
        //old_ptr->size = size;
        return oldp;
    }
    void* new_ptr = smalloc(size);
    if(nullptr == new_ptr){
        return nullptr;
    }
    memcpy(new_ptr, oldp, old_ptr->size);
    sfree(oldp);
    return new_ptr;
}

size_t _num_free_blocks(){
    size_t count = 0;
    metadata it = first;
    while(nullptr != it){
        if(it->is_free){
            count++;
        }
        it = it->next;
    }
    return count;
}

size_t _num_free_bytes(){
    size_t count = 0;
    metadata it = first;
    while(nullptr != it){
        if(it->is_free){
            count += it->size;
        }
        it = it->next;
    }
    return count;
}

size_t _num_allocated_blocks(){
    size_t count = 0;
    metadata it = first;
    while(nullptr != it){
        count++;
        it = it->next;
    }
    return count;
}

size_t _num_allocated_bytes(){
    size_t count = 0;
    metadata it = first;
    while(nullptr != first){
        count += it->size;
        it = it->next;
    }
    return count;
}

size_t _num_meta_data_bytes(){
    size_t count = 0;
    metadata it = first;
    while(nullptr != it){
        count += sizeof(struct metadata_t);
        it = it->next;
    }
    return count;
}

size_t _size_meta_data(){
    return sizeof(struct metadata_t);
}

