#include "HeapAllocator.h"
#include "Block.h"
#include <iostream>
#include <cstdint>
using namespace std;

// 1 MB heap size
const size_t HEAP_SIZE = 1024 * 1024;
static uint8_t heap[HEAP_SIZE]; 

// POINTERS
// heapStart: Points to the FIRST block in physical memory (start of the heap).
//            Used for debug traversal and bounds checking.
Block* heapStart = nullptr;

// freeList: Points to the FIRST free block. 
//           We only search this list for allocations.
Block* freeList = nullptr;

// Global Strategy (Default: FIRST_FIT)
AllocStrategy currentStrategy = AllocStrategy::FIRST_FIT;

void setStrategy(AllocStrategy strategy) {
    currentStrategy = strategy;
    cout << "Allocation Strategy Switched to: " 
              << (strategy == AllocStrategy::FIRST_FIT ? "First Fit" : "Best Fit") 
              << "\n";
}

void initHeap() {
    // 1. Initialize heapStart
    heapStart = reinterpret_cast<Block*>(heap);
    
    // 2. Initialize the first block
    heapStart->size = HEAP_SIZE - sizeof(Block);
    heapStart->free = true;

    // Physical Pointers (No neighbors yet)
    heapStart->next = nullptr;
    heapStart->prev = nullptr;

    // Free List Pointers (Only one free block)
    heapStart->nextFree = nullptr;
    heapStart->prevFree = nullptr;

    // 3. Point freeList to this block
    freeList = heapStart;

    cout << "Heap initialized at: " << (void*)heap << "\n";
    cout << "Initial free block size: " << freeList->size << " bytes\n";
    cout << "Block metadata overhead: " << sizeof(Block) << " bytes\n";
}

// Align size to 8 bytes
size_t align8(size_t size) {
    return (size + 7) & ~7;
}

// Remove a block from the free list
void removeFree(Block* b) {
    if (!b) return;
    if (b->prevFree) {
        b->prevFree->nextFree = b->nextFree;
    } else {
        // b was the head
        freeList = b->nextFree;
    }
    
    if (b->nextFree) {
        b->nextFree->prevFree = b->prevFree;
    }

    // Clean up pointers for safety 
    b->nextFree = nullptr;
    b->prevFree = nullptr;
}

// Insert a block into the free list (at head)
void insertFree(Block* b) {
    if (!b) return;
    
    b->nextFree = freeList;
    b->prevFree = nullptr;
    
    if (freeList) {
        freeList->prevFree = b;
    }
    
    freeList = b;
}

void* my_malloc(size_t size) {
    size = align8(size);
    
    Block* curr = freeList;
    Block* target = nullptr;

    if (!heapStart) initHeap();

    // 1. Search (Only traverse freeList)
    if (currentStrategy == AllocStrategy::FIRST_FIT) {
        while (curr != nullptr) {
            if (curr->size >= size) {
                target = curr;
                break;
            }
            curr = curr->nextFree;
        }
    } else {
        // BEST FIT
        Block* best = nullptr;
        while (curr != nullptr) {
            if (curr->size >= size) {
                if (!best || curr->size < best->size) {
                    best = curr;
                }
            }
            curr = curr->nextFree;
        }
        target = best;
    }

    if (!target) {
        cout << "Out of memory!\n";
        return nullptr;
    }

    // 2. Allocation
    // We found a target. It is currently in the free list.
    
    // Check split
    if (target->size >= size + sizeof(Block)) {
        // SPLIT
        // a. Create new block physically after target
        Block* newBlock = (Block*)((uint8_t*)target + sizeof(Block) + size);
        
        newBlock->size = target->size - size - sizeof(Block);
        newBlock->free = true;
        
        // b. Update Physical Pointers
        newBlock->next = target->next;
        newBlock->prev = target;
        if (target->next) {
            target->next->prev = newBlock;
        }
        target->next = newBlock;

        // c. Update Free List
        // Instead of remove(target) + insert(newBlock), we can just replace target with newBlock in-place
        // in the free list to be efficient!
        newBlock->nextFree = target->nextFree;
        newBlock->prevFree = target->prevFree;
        
        if (target->prevFree) target->prevFree->nextFree = newBlock;
        else freeList = newBlock;
        
        if (target->nextFree) target->nextFree->prevFree = newBlock;

        // d. Finalize target
        target->size = size;
        target->free = false;
        target->nextFree = nullptr;
        target->prevFree = nullptr;

        cout << "  [Alloc] Split block at " << target << " (Used: " << size << ", New Free: " << newBlock->size << ")\n";
    } else {
        // NO SPLIT
        // Just remove target from free list
        removeFree(target);
        target->free = false;
        cout << "  [Alloc] Taken whole block at " << target << " (Size: " << target->size << ")\n";
    }

    return (void*)(target + 1);
}

void my_free(void* ptr) {
    if (!ptr) return;

    // Bound check
    if ((uint8_t*)ptr < heap || (uint8_t*)ptr >= heap + HEAP_SIZE) {
        cout << "ERROR: Invalid pointer\n";
        return;
    }

    Block* curr = (Block*)ptr - 1;

    if (curr->free) {
        cout << "ERROR: Double Free\n";
        return;
    }

    curr->free = true;
    cout << "Freed block at " << curr << "\n";

    // 1. Initial Insert (We will adjust this if we coalesce)
    // Actually, checking neighbors first is easier.
    
    // Coalesce NEXT (Physical)
    if (curr->next && curr->next->free) {
        Block* nextParams = curr->next;
        cout << "  [Coalesce] Merging with NEXT block at " << nextParams << "\n";
        
        // Remove 'nextParams' from free list (it's being eaten)
        removeFree(nextParams);
        
        // Merge sizes
        curr->size += sizeof(Block) + nextParams->size;
        
        // Update physical links
        curr->next = nextParams->next;
        if (curr->next) {
            curr->next->prev = curr;
        }
    }

    // Coalesce PREV (Physical)
    if (curr->prev && curr->prev->free) {
        Block* prevParams = curr->prev;
        cout << "  [Coalesce] Merging with PREV block at " << prevParams << "\n";
        
        // 'prevParams' is ALREADY in the free list.
        // We just expand it to eat 'curr'.
        // So we do NOT need to insert 'curr' into the free list.
        
        prevParams->size += sizeof(Block) + curr->size;
        
        // Update physical links
        prevParams->next = curr->next;
        if (prevParams->next) {
            prevParams->next->prev = prevParams;
        }
        
        // 'prevParams' stays in free list. We are done.
    } else {
        // If we didn't merge with Prev, we MUST insert 'curr' into the free list ourselves.
        insertFree(curr);
    }
}

void debugHeap() {
    cout << "\n--- Heap Statistics ---\n";
    
    // We traverse PHYSICAL blocks to see everything
    Block* curr = heapStart;
    
    int totalBlocks = 0;
    int freeBlocks = 0;
    int usedBlocks = 0;
    size_t totalFreeSize = 0;
    size_t totalUsedSize = 0;
    size_t largestFreeBlock = 0;

    while (curr != nullptr) {
        totalBlocks++;
        if (curr->free) {
            freeBlocks++;
            totalFreeSize += curr->size;
            if (curr->size > largestFreeBlock) largestFreeBlock = curr->size;
        } else {
            usedBlocks++;
            totalUsedSize += curr->size;
        }
        
        cout << "  Block " << (totalBlocks - 1) << ": Addr=" << curr 
             << " Size=" << curr->size 
             << " Status=" << (curr->free ? "FREE" : "USED") << "\n";
             
        curr = curr->next;
    }

    double fragmentation = 0.0;
    if (totalFreeSize > 0) {
        fragmentation = 1.0 - ((double)largestFreeBlock / (double)totalFreeSize);
    }
    
    cout << "\n--- Summary ---\n";
    cout << "Total Blocks:     " << totalBlocks << "\n";
    cout << "Used Blocks:      " << usedBlocks << " (Size: " << totalUsedSize << ")\n";
    cout << "Free Blocks:      " << freeBlocks << " (Size: " << totalFreeSize << ")\n";
    cout << "Largest Free Blk: " << largestFreeBlock << "\n";
    cout << "Fragmentation:    " << (fragmentation * 100.0) << "%\n";
    cout << "-----------------------\n\n";
}

