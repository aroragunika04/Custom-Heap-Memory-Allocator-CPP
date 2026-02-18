#pragma once
#include <cstddef>

// Initialize the private heap
void initHeap();

// Allocate memory (First Fit)
void* my_malloc(size_t size);

// Deallocate memory (with Coalescing)
void my_free(void* ptr);

// Print heap statistics
void debugHeap();

enum class AllocStrategy {
    FIRST_FIT,
    BEST_FIT
};

void setStrategy(AllocStrategy strategy);
