#include <math.h>
#include <cstring>
#include <unistd.h>
#include <sys/mman.h>

using namespace std;

//size of this struct is 32
typedef struct metadata_t {
    size_t size;
    bool is_free;
    bool is_sbrk;
    metadata_t* next;
    metadata_t* prev;
} *metadata;

//sbrk list
metadata first = nullptr;
metadata last = nullptr;

//mmap list
metadata first_mmap = nullptr;
metadata last_mmap = nullptr;

const int large_enough_size = 128;
const int size_to_mmap = 128000; //128KB

static void challenge1_block_cutter(metadata block_to_split, size_t size);
static metadata merge_with_prev(metadata p);
static metadata merge_with_next(metadata p);
static metadata challenge2_block_combine(metadata p);
static void* challenge4_mmap_block(size_t size);
static inline bool size_check(metadata t, size_t size){
    return t->size + t->prev->size + t->next->size + 2*sizeof(struct metadata_t) >= size;
}

static inline size_t align_size(size_t size){
    return size % 8 == 0 ? size : size + 8 - size % 8;
}

void* smalloc(size_t size1){
    if(size1 <= 0 || size1 > (size_t)(pow(10.0,8.0))){
       return nullptr;
    }
    size_t size = align_size(size1);
    if(size >= size_to_mmap){
        return challenge4_mmap_block(size);
    }
    if (first == nullptr || (first->size >= size && first->is_free)){
        if(nullptr != first && (first->size >= size + sizeof(struct metadata_t) + large_enough_size)){
            challenge1_block_cutter(first, size);
            return (first + 1);
        }
        if(nullptr != first && first->size >= size && first->is_free){
            first->is_free = false;
            //first->size = size;
            return (first + 1);
        }
        metadata alloc_ptr = (metadata)sbrk(0);
        void* new_block = sbrk(size + sizeof(struct metadata_t));
        if(new_block == (void*)(-1)){
            return nullptr;
        }
        if(last == nullptr){
            last = alloc_ptr;
        }
        if(first == nullptr){
            alloc_ptr->next = nullptr;
            first = alloc_ptr;
        }
        alloc_ptr->size = size;
        alloc_ptr->is_free = false;
        alloc_ptr->is_sbrk = true;
        alloc_ptr->prev = nullptr;
        return (alloc_ptr + 1);
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
            if(it->size < size){
                size_t delta = size - it->size;
                void* ptr = sbrk(delta);
                if(ptr == (void*)(-1)){
                    return nullptr;
                }
                it->size = size;
                it->is_free = false;
                return (it + 1);
            } else {
                it->size = size;
                it->is_free = false;
                return (it + 1);
            }
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
        return (alloc_ptr + 1);
    }
    if(it->next->size >= size + sizeof(struct metadata_t) + large_enough_size){
        //TODO: call challenge1 function
        challenge1_block_cutter(it->next, size);
        return (it->next + 1);
    }
    it->next->is_free = false;
    it->next->size = size;
    return (it->next + 1);
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
    metadata met = (metadata)p - 1;
    if(!(met->is_sbrk)){
        if(first_mmap == met){
            if(first_mmap == last_mmap){
                first_mmap = nullptr;
                last_mmap = nullptr;
            } else {
                first_mmap = met->next;
            }
            munmap(p, met->size + sizeof(struct metadata_t));
            return;
        }
        if(last_mmap == met){
            last_mmap = met->prev;
        }
        if(nullptr != met->prev){
            met->prev->next = met->next;
        }
        if(nullptr != met->next){
            met->next->prev = met->prev;
        }
        munmap(p, met->size + sizeof(struct metadata_t));
        return;
    }
    met->is_free = true;
    challenge2_block_combine(met);
}

void* srealloc(void* oldp, size_t size1){
    if(size1 <= 0 || size1 > (size_t)(pow(10.0, 8.0))){
        return nullptr;
    }
    if(nullptr == oldp){
        return smalloc(size1);
    }
    size_t size = align_size(size1);
    metadata old_ptr = (metadata)oldp - 1;
    if(!old_ptr->is_sbrk){
        void* new_block = challenge4_mmap_block(size);
        if(nullptr == new_block){
            return nullptr;
        }
        //TODO: check how many bytes to copy
        size_t minimum = old_ptr->size < size ? old_ptr->size : size;
        memmove(new_block, oldp, minimum); //TODO: check memove ?
        sfree(oldp);
        return new_block;
    }
    if((old_ptr == last || nullptr == old_ptr->next) && old_ptr->size < size){
        size_t delta = size - old_ptr->size;
        if(sbrk(delta) == (void*)(-1)){
            return nullptr;
        }
        old_ptr->size = size;
        return oldp;
    }
    if(old_ptr->size >= size){
        if(old_ptr->size >= size + sizeof(struct metadata_t) + large_enough_size){
            challenge1_block_cutter(old_ptr, size);
        }
        return oldp;
    }
    if(nullptr != old_ptr->prev && old_ptr->prev->is_free && old_ptr->size + old_ptr->prev->size >= size){
        old_ptr->is_free = true;
        old_ptr = merge_with_prev(old_ptr);
        old_ptr->is_free = false;
        old_ptr->is_sbrk = true;
        //old_ptr->size -= sizeof(struct metadata_t);
        if(old_ptr->size >= size + sizeof(struct metadata_t) + large_enough_size){
            challenge1_block_cutter(old_ptr, size);
        }
        memmove(old_ptr + 1, oldp, old_ptr->size < size ? old_ptr->size : size);
        return (old_ptr + 1);
    }
    if(nullptr != old_ptr->next && old_ptr->next->is_free && old_ptr->size + old_ptr->next->size >= size){
        old_ptr->is_free = true;
        old_ptr = merge_with_next(old_ptr);
        old_ptr->is_free = false;
        old_ptr->is_sbrk = true;
        //old_ptr->size -= sizeof(struct metadata_t);
        if(old_ptr->size >= size + sizeof(struct metadata_t) + large_enough_size){
            challenge1_block_cutter(old_ptr, size);
        }
        memmove(old_ptr + 1, oldp, old_ptr->size < size ? old_ptr->size : size);
        return (old_ptr + 1);
    }
    if(nullptr != old_ptr->prev && nullptr != old_ptr->next && old_ptr->prev->is_free && old_ptr->next->is_free){
        if(size_check(old_ptr, size)){
            old_ptr->is_free = true;
            old_ptr = challenge2_block_combine(old_ptr);
            old_ptr->is_free = false;
            old_ptr->is_sbrk = true;
            if(old_ptr->size >= size + sizeof(struct metadata_t) + large_enough_size){
                challenge1_block_cutter(old_ptr, size);
            }
            //old_ptr->size -= sizeof(struct metadata_t);
            memmove(old_ptr + 1, oldp, old_ptr->size < size ? old_ptr->size : size);
            old_ptr->is_free = false;
            return (old_ptr + 1);
        }
    }
    old_ptr->is_free = true;
    void* new_ptr = smalloc(size);
    if(nullptr == new_ptr){
        old_ptr->is_free = false;
        return nullptr;
    }
    if(new_ptr == oldp){
        return new_ptr;
    }
    size_t minimum = old_ptr->size < size ? old_ptr->size : size;
    memmove(new_ptr, oldp, minimum); 
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
    metadata it2 = first_mmap;
    while(nullptr != it2){
        count++;
        it2 = it2->next;
    }
    return count;
}

size_t _num_allocated_bytes(){
    size_t count = 0;
    metadata it = first;
    while(nullptr != it){
        count += it->size;
        it = it->next;
    }
    metadata it2 = first_mmap;
    while(nullptr != it2){
        count += it2->size;
        it2 = it2->next;
    }
    return count;
}

size_t _num_meta_data_bytes(){
    /*size_t count = 0;
    metadata it = first;
    while(nullptr != it){
        count += sizeof(struct metadata_t);
        it = it->next;
    }
    metadata it2 = first_mmap;
    while(nullptr != it2){
        count += sizeof(struct metadata_t);
        it2 = it2->next;
    }
    return count;*/
    return _num_allocated_blocks() * sizeof(struct metadata_t);
}

size_t _size_meta_data(){
    return sizeof(struct metadata_t);
}

//Challenge 1

static void challenge1_block_cutter(metadata block_to_split, size_t size){
    //2nd half should be 'sizeof(metadata) + size' space after the 1st half 
    //why char* ? because sizeof(char) is 1 and we want to advance in size bytes
    
    char* temp = (char*)(block_to_split);
    metadata second_half = (metadata)(temp + sizeof(struct metadata_t) + size);
    
    second_half->is_free = true;
    second_half->is_sbrk = true;
    second_half->size = align_size(block_to_split->size - size - sizeof(struct metadata_t));
    second_half->next = block_to_split->next;
    second_half->prev = block_to_split;
    
    block_to_split->is_sbrk = true;
    block_to_split->size = align_size(size);
    block_to_split->next = second_half;
    block_to_split->is_free = false;
    
    //according to the PDF, if second half and its next are free we should NOT merge them!
    /*if(second_half->is_free && nullptr != second_half->next && second_half->next->is_free){
        //TODO: merge blocks. call challenge 2 function
        merge_with_next(second_half);
    }*/
}

//Challenge 2

static metadata merge_with_prev(metadata p){
    if(nullptr == p || nullptr == p->prev){
        return nullptr;
    }
    if(!p->is_free || !p->prev->is_free){
        return nullptr;
    }
    p->prev->size += (p->size + sizeof(struct metadata_t));
    //metadata temp = p;
    p->prev->next = p->next;
    if(nullptr != p->next){
        p->next->prev = p->prev;
    }
    return p->prev;
}

static metadata merge_with_next(metadata p){
    if(nullptr == p || nullptr == p->next){
        return nullptr;
    }
    if(!p->is_free || !p->next->is_free){
        return nullptr;
    }
    p->size += (p->next->size + sizeof(struct metadata_t));
    metadata temp = p->next;
    p->next = p->next->next;
    if(nullptr != temp->next){
        temp->next->prev = p;
    }
    return p;
}

static metadata challenge2_block_combine(metadata p){
    merge_with_next(p);
    return merge_with_prev(p) ? p->prev : p;
}
  //Challenge 4
  
static void* challenge4_mmap_block(size_t size1){
    size_t size = align_size(size1);
    char* map = (char*)mmap(nullptr, size + sizeof(struct metadata_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if(map == MAP_FAILED){
        return nullptr;
    }
    metadata m = (metadata)map;
    if(nullptr == first_mmap){
        first_mmap = m;
        m->prev = nullptr;
    }
    m->is_free = false;
    m->is_sbrk = false;
    if(last_mmap != nullptr){
        m->prev = last_mmap;
        last_mmap->next = m;
    }
    m->next = nullptr;
    m->size = size;
    last_mmap = m;
    return (void*)(map + sizeof(struct metadata_t));
}

