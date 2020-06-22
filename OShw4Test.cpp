#include<iostream>
#include<cassert>
#include "malloc_3.cpp"


#define META_SIZE         sizeof(MallocMetadata) // put your meta data name here

int main() {
    void *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8, *p9, *p10, *p11, *p12, *p13, *p14 ,*p15;
    //check mmap allocation and malloc/calloc fails
    smalloc(0);
    smalloc(100000001);
    scalloc(10000000, 10000000);
    scalloc(100000001, 1);
    scalloc(1, 100000001);
    scalloc(50, 0);
    scalloc(0, 50);

    p1 = smalloc(100000000);
    p2 = scalloc(1, 10000000);
    p3 = scalloc(10000000, 1);
    p1 = srealloc(p1, 100000000);
    p3 = srealloc(p3, 20000000);
    p3 = srealloc(p3, 10000000);
    p4 = srealloc(nullptr, 10000000);
    assert(_num_free_blocks() == 0);
    assert(_num_free_bytes() == 0);
    assert(_num_allocated_blocks() == 4);
    assert(_num_allocated_bytes() == 100000000 + 30000000);
    assert(_num_meta_data_bytes() == 4 * META_SIZE);
    sfree(p1), sfree(p2), sfree(p3), sfree(p4);
    assert(_num_free_blocks() == 0);
    assert(_num_free_bytes() == 0);
    assert(_num_allocated_blocks() == 0);
    assert(_num_allocated_bytes() == 0);
    assert(_num_meta_data_bytes() == 0);
    p1 = smalloc(1000);
    p2 = smalloc(1000);
    p3 = smalloc(1000);
    p4 = scalloc(500,2);
    p5 = scalloc(2, 500);
    assert(_num_free_blocks() == 0);
    assert(_num_free_bytes() == 0);
    assert(_num_allocated_blocks() == 5);
    assert(_num_allocated_bytes() == 5000);
    assert(_num_meta_data_bytes() == 5 * META_SIZE);
    //check free, combine and split
    sfree(p3); sfree(p5); sfree(p4);
    cout << _num_free_blocks() << endl;
    assert(_num_free_blocks() == 1);
    p3 = smalloc(1000); p4 = smalloc(1000); p5 = smalloc (1000);
    sfree(p4); sfree(p5);
    sfree(p1); sfree(p1); sfree(p2);
    p1 = smalloc(1000); p2 = smalloc(1000);
    sfree(p2); sfree(p1);
    cout << _num_free_blocks() << endl;
    assert(_num_free_blocks() == 2);
    assert(_num_free_bytes() == 4000 + 2*META_SIZE);
    assert(_num_allocated_blocks() == 3);
    assert(_num_allocated_bytes() == 5000 + 2*META_SIZE);
    assert(_num_meta_data_bytes() == 3 * META_SIZE);
//    assert(block_list.tail == (MallocMetadata*)p4 - 1);
//    assert(block_list.head == (MallocMetadata*)p1 - 1);
    //list condition: FREE OF 2032 -> BLOCK OF 1000 -> FREE OF 2032
    p1 = smalloc(1000);
    p2 = smalloc(1000);
    p4 = scalloc(1500,2);
    assert(_num_free_blocks() == 0);
    assert(_num_free_bytes() == 0);
    assert(_num_allocated_blocks() == 4);
    assert(_num_allocated_bytes() == 6000);
    assert(_num_meta_data_bytes() == 4 * META_SIZE);
  //  assert(block_list.tail == (MallocMetadata*)p4 - 1);
   // assert(block_list.head == (MallocMetadata*)p1 - 1);
    sfree(p1); sfree(p2); sfree(p3); sfree(p4);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 6000+3*META_SIZE);
    assert(_num_allocated_blocks() == 1);
    assert(_num_allocated_bytes() == 6000+3*META_SIZE);
    assert(_num_meta_data_bytes() == META_SIZE);
//    assert(block_list.tail == (MallocMetadata*)p1 - 1);
//    assert(block_list.head == (MallocMetadata*)p1 - 1);
//    assert(mmap_list.head == nullptr);
//    assert(mmap_list.head == nullptr);
    p1 = smalloc(1000); p2 = scalloc(2,500);
    p3 = smalloc(1000); p4 = scalloc(10,100);
    p5 = smalloc(1000); p6 = scalloc(250,4);

    //list condition: SIX BLOCKS, 1000 BYTES EACH
    //now we'll check realloc

    //check englare
    sfree(p5);
    for (int i = 0; i < 250; ++i)
        *((int*)p6+i) = 2;
    p6 = srealloc(p6, 2000);
    for (int i = 0; i < 250; ++i)
        assert(*((int*)p6+i) == 2);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 1000);
    assert(_num_allocated_blocks() == 6);
    assert(_num_allocated_bytes() == 7000);
    assert(_num_meta_data_bytes() == 6*META_SIZE);
//    assert(block_list.tail == (MallocMetadata*)p6 - 1);
//    assert(block_list.head == (MallocMetadata*)p1 - 1);
    p5 = smalloc(1000);
    p1 = srealloc(p1,500-sizeof(MallocMetadata));
    sfree(p1);
    //check case a

    assert(((MallocMetadata*)p5-1)->is_free==false);
    sfree(p5); sfree(p3);
    //check case b
    p3 = srealloc(p4,2000);
    //LIST CONDITION: 1-> 1000 FREE, 2->1000, 3->2000,
    // 5-> 1000 FREE, 6->2000 FREE
    //now i return the list to its state before tha last realloc
    p3 = srealloc(p3,1000);
    p4 = ((char*)p3 + 1000 + META_SIZE);
    p1 = smalloc(1000); p4=smalloc(1000); sfree(p1);
    sfree(p3);
    //check case d
    p3 = srealloc(p4,3000);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 1000);
    assert(_num_allocated_blocks() == 4);
    assert(_num_allocated_bytes() == 7000 + 2*META_SIZE);
    assert(_num_meta_data_bytes() == 4*META_SIZE);
    //just change the state of the list
    p3 = srealloc(p3,500);
    p1 = smalloc(1000);
    p4 = smalloc(1500);
    p5 = smalloc(1000);
    sfree(p1); sfree(p3); sfree(p5);
    //check case c
    p4 = srealloc(p4, 2500);
    p4 = srealloc(p4, 1000);

    assert(_num_free_blocks() == 3);
    assert(_num_free_bytes() == 3000);
    assert(_num_allocated_blocks() == 6);
    assert(_num_allocated_bytes() == 7000);
    assert(_num_meta_data_bytes() == 6*META_SIZE);
    //just change the state of the list
    p1 = smalloc(1000);
    p3 = smalloc(500);
    sfree(p1);
    //check case e
    p1 = srealloc(p3, 900);
    assert(_num_free_blocks() == 2);
    assert(_num_free_bytes() == 2000);
    assert(_num_allocated_blocks() == 6);
    assert(_num_allocated_bytes() == 7000);
    assert(_num_meta_data_bytes() == 6*META_SIZE);
  //  assert(block_list.tail == (MallocMetadata*)p6 - 1);
   // assert(block_list.head == (MallocMetadata*)p1 - 1);
    //check case f
    for (int i = 0; i < 250; ++i)
        *((int*)p1+i) = 2;
    p7 = srealloc(p1,2000);
    for (int i = 0; i < 250; ++i)
        assert(*((int*)p7+i) == 2);
    assert(_num_free_blocks() == 3);
    assert(_num_free_bytes() == 3000);
    assert(_num_allocated_blocks() == 7);
    assert(_num_allocated_bytes() == 9000);
    assert(_num_meta_data_bytes() == 7*META_SIZE);
   // assert(block_list.tail == (MallocMetadata*)p7 - 1);
   // assert(block_list.head == (MallocMetadata*)p1 - 1);
    //block_list.print_list();
    /*LIST CONDITION:
     * 1->1000 free
     * 2->1000
     * 3->500 free
     * 4->1000
     * 5->1500 free
     * 6->2000
     * 7->2000
    */
    cout << "good job!" << endl;
    return 0;
}
