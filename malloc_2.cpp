//#ifndef _malloc_2
//#define _malloc_2

#include <math.h>
#include <cstring>
#include <unistd.h>
#include <cassert>
#include <iostream>

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
    return(void*)(it->next + sizeof(struct metadata_t));
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
    /*size_t count = 0;
    metadata it = first;
    while(nullptr != it){
        count += sizeof(struct metadata_t);
        it = it->next;
    }
    return count;*/
    //much more elegant:
    return _num_allocated_blocks() * sizeof(struct metadata_t);
}

size_t _size_meta_data(){
    return sizeof(struct metadata_t);
}

//#endif //_malloc_2
//TEST: TODO remove before submission!
typedef struct metadata_t MallocMetadata;

int main() {
    std::cout <<sizeof(MallocMetadata)<<std::endl;
    cout << "1" << endl;
    std::cout <<smalloc(10)<<std::endl;
cout << "2" << endl;
    int* ptr = (int*)smalloc(32);
    for (int i = 0; i < (32 / sizeof(int)); i++) {
        ptr[i] = i;
    }

    for (int i = 0; i < (32 / sizeof(int)); i++) {
        cout << ptr[i] << "  "; // needs to print 0 1 2 3 4 5 6 7
    }
    cout << endl;

    int* ptr2 = (int*) srealloc(ptr, 32);
    if (ptr2 != ptr) cout << "not suppose to happen";

    for (int i = 0; i < (32 / sizeof(int)); i++) {
        if (ptr2[i] != i) {
            cout << "You have a mistake you stupid bronze 1 ";
            cout << "expectd: " << i << " got: " << ptr2[i] << endl;
        }
    }
    cout << endl;

    int* ptrnew = (int*)smalloc(32);
    if (ptrnew == ptr) cout << "your programming sucks. you free'd ptr so now ptrnew needs to get the same adrress..." << endl;

    std::cout <<ptr<<std::endl;
    sfree (ptr);
    ptr = (int*)smalloc(15);
cout << "3" << endl;
    std::cout <<ptr<<std::endl;
    memset(ptr, 5 ,15);
    //std::cout <<"nside ptr  :"<<((int)ptr)<<std::endl;
    //std::cout <<"size ptr"<<alocationList->getSize(ptr)<<std::endl;
    cout << "4" << endl;
    std::cout << srealloc(ptr,7) << std::endl;
cout << "5" << endl;
    std::cout << srealloc(ptr,25) << std::endl;
cout << "6" << endl;
    std::cout <<smalloc(18)<<std::endl;
cout << "7" << endl;
    ptr = (int*) scalloc(4,6);

    std::cout <<ptr<<std::endl;

    char* ptr3 = (char*)scalloc(1, 10);
    for (int i = 0; i < 10; i++) {
        if (ptr3[i] != 0) cout << "WTF you have a problem. A sheep programming for ISIS does a better job than you";
    }
//    std::cout <<"size ptr"<<alocationList->getSize(ptr)<<std::endl;
    return 0;
}
