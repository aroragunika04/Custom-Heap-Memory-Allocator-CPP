#pragma once
#include <cstddef>

struct Block {
    size_t size;     
    bool free;       

    // --- HEAP LIST POINTERS ---
    // These link ALL blocks (used and free) in memory order.
    // Used for coalescing (merging with neighbors).
    Block* next; 
    Block* prev; 

    // --- FREE LIST POINTERS  ---
    // These link ONLY free blocks. 
    Block* nextFree;
    Block* prevFree;
};
