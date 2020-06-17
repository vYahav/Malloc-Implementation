#include <math.h>
#include <cstring>
#include <unistd.h>

using namespace std;

typedef struct metadata_t {
    size_t size;
    bool is_free;
    bool is_sbrk;
    metadata_t* next;
    metadata_t* prev;
} *metadata;

metadata first = nullptr;
metadata last = nullptr;

const int large_enough_size = 128;
const int size_to_mmap = 128000; //128KB
void challenge1_block_cutter(metadata block_to_split, size_t size);
void merge_with_prev(metadata p);
void merge_with_next(metadata p);
void challenge2_block_combine(metadata p);
//TODO: check if sizeof(ptr + struct metadata_t) OR sizeof(ptr + 1)

void* smalloc(size_t size){
    if(size <= 0 || size > (size_t)(pow(10.0,8.0))){
        return nullptr;
    }
    if (first == nullptr || (first->size >= size && first->is_free)){
        //TODO: check if allocation really puts metadata first!!
        if(nullptr != first && (first->size >= size + sizeof(struct metadata_t) + large_enough_size)){
            //TODO: call challenge1 funnction
            challenge1_block_cutter(first, size);
            return (void*)(first + sizeof(struct metadata_t));
        }
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
        alloc_ptr->is_sbrk = true;
        alloc_ptr->prev = nullptr;
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
        if(it->is_free){ //challenge3 case
            size_t delta = size - it->size;
            void* ptr = sbrk(delta);
            if(ptr == (void*)(-1)){
                return nullptr;
            }
            it->size += delta;
            it->is_free = false;
            return (void*)(it + sizeof(struct metadata_t));
        }
        //meaning we reached end of our allocation list
        metadata alloc_ptr = (metadata)sbrk(0);
        void* new_block = sbrk(size + sizeof(struct metadata_t));
        if(new_block == (void*)(-1)){
            return nullptr;
        }
        last = alloc_ptr;
        it->next = alloc_ptr;
        alloc_ptr->size = size;
        alloc_ptr->is_sbrk = true;
        alloc_ptr->is_free = false;
        alloc_ptr->prev = it;
        alloc_ptr->next = nullptr;
        return (void*)(alloc_ptr + sizeof(struct metadata_t));
    }
    if(it->next->size >= size + sizeof(struct metadata_t) + large_enough_size){
        //TODO: call challenge1 function
        challenge1_block_cutter(it->next, size);
        return(void*)(it->next + sizeof(struct metadata_t));
    }
    it->next->is_free = false;
    it->next->size = size;
    return (void *)(it->next + sizeof(struct metadata_t));
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
    challenge2_block_combine(met);
}

void* srealloc(void* oldp, size_t size){
    if(size <= 0 || size > (size_t)(pow(10.0, 8.0))){
        return nullptr;
    }
    if(nullptr == oldp){
        return smalloc(size);
    }
    metadata old_ptr = (metadata)oldp - sizeof(struct metadata_t);
    //if(old_ptr->is_free){ TODO: check if we need to realloc an unallocated pointer!
    //  return nullptr;
    //}
    if(old_ptr->size >= size){
        //old_ptr->size = size; TODO: check this!!
        if(old_ptr->size >= size + sizeof(struct metadata_t) + large_enough_size){
            challenge1_block_cutter(old_ptr, size);
        }
        return oldp;
    }
    if(nullptr != old_ptr->prev && old_ptr->prev->is_free &&  old_ptr->size + old_ptr->prev->size >= size){
        merge_with_prev(old_ptr);
        if(old_ptr->size >= size + sizeof(struct metadata_t) + large_enough_size){
            challenge1_block_cutter(old_ptr, size);
        }
        return oldp;
    }
    if(nullptr != old_ptr->next && old_ptr->next->is_free && old_ptr->size + old_ptr->next->size >= size){
        merge_with_next(old_ptr);
        if(old_ptr->size >= size + sizeof(struct metadata_t) + large_enough_size){
            challenge1_block_cutter(old_ptr, size);
        }
        return oldp;
    }
    if(nullptr != old_ptr->prev && nullptr != old_ptr->next && old_ptr->prev->is_free && old_ptr->next->is_free){
        if(old_ptr->size + old_ptr->prev->size + old_ptr->next->size >= size){
            challenge2_block_combine(old_ptr);
        }
        if(old_ptr->size >= size + sizeof(struct metadata_t) + large_enough_size){
            challenge1_block_cutter(old_ptr, size);
        }
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

//Challenge 1

void challenge1_block_cutter(metadata block_to_split, size_t size){
    //2nd half should be 'sizeof(metadata) + size' space after the 1st half 
    //why char* ? because sizeof(char) is 1 and we want to advance in size bytes
    metadata second_half = (metadata)((char*)block_to_split + sizeof(struct metadata_t) + size);
    
    second_half->is_free = true;
    second_half->size = block_to_split->size - size - sizeof(struct metadata_t);
    second_half->next = block_to_split->next;
    second_half->prev = block_to_split;
    
    block_to_split->size = size;
    block_to_split->next = second_half;
    block_to_split->is_free = false;

    //TODO: if splitted->next is free than we should connect them together to 1 block
    //so we need to call chllenge 2 function
    if(second_half->is_free && second_half->next->is_free){
        //TODO: merge blocks. call challenge 2 function
        merge_with_next(second_half);
    }
}

//Challenge 2

void merge_with_prev(metadata p){
    if(nullptr == p || nullptr == p->prev){
        return;
    }
    if(!p->is_free || !p->prev->is_free){
        return;
    }
    p->prev->size += p->size + sizeof(struct metadata_t);
    p->prev->next = p->next;
    if(nullptr != p->next->prev){
        p->next->prev = p->prev;
    }
}

void merge_with_next(metadata p){
    if(nullptr == p || nullptr == p->next){
        return;
    }
    if(!p->is_free || !p->next->is_free){
        return;
    }
    p->size += p->next->size + sizeof(struct metadata_t);
    p->next = p->next->next;
    if(nullptr != p->next->next){
        p->next->next->prev = p;
    }
}

void challenge2_block_combine(metadata p){
    merge_with_prev(p);
    merge_with_next(p);
}
  //Challenge 3
  































