#include "allocator/HeapAllocator.h"
#include <iostream>
using namespace std;

int main() {
    cout << "Custom Malloc Project Started...\n";
    initHeap();
    debugHeap();

    cout << "\n=== Test 1: Basic Allocation ===\n";
    void* p1 = my_malloc(100);
    debugHeap();

    cout << "\n=== Test 2: Split Allocation ===\n";
    void* p2 = my_malloc(200); // Should split the remaining big block
    debugHeap();

    cout << "\n=== Test 3: Free & Coalesce (Sandwich) ===\n";
    // We have p1 (100 used), p2 (200 used), and rest (free).
    // Let's alloc one more to trap p2 in the middle of used blocks if we wanted, 
    // but let's stick to the user's sandwich request nicely.
    
    void* p3 = my_malloc(100);
    debugHeap();

    cout << "Freeing p1 (Left)..." << endl;
    my_free(p1);
    debugHeap();

    cout << "Freeing p3 (Right)..." << endl;
    my_free(p3);
    debugHeap();

    cout << "Freeing p2 (Middle) -> Should merge Left(p1) and Right(p3)..." << endl;
    my_free(p2);
    debugHeap();

    cout << "\n=== Test 4: Error Handling ===\n";
    cout << "1. Double Free:\n";
    my_free(p2); // Already freed

    cout << "2. Invalid Pointer:\n";
    int stackVar = 10;
    my_free(&stackVar);

    // --- BEST FIT DEMO ---
    cout << "\n=== Test 5: Best Fit Strategy ===\n";
    
    // 1. Reset everything (or just free what we have)
    // Actually, let's just use the remaining heap.
    
    // Create a specific fragmentation scenario
    // [ 100 used ] [ 500 free ] [ 100 used ] [ 200 free ] [ 100 used ] ...
    
    cout << "Setting up Best Fit Scenario...\n";
    void* a = my_malloc(100);
    void* bigHole = my_malloc(500); 
    void* b = my_malloc(100);
    void* smallHole = my_malloc(200); 
    void* c = my_malloc(100);
    
    // Free the holes to create gaps
    my_free(bigHole);
    my_free(smallHole);
    
    cout << "\nCurrent Heap State (Expect hole of 500 and hole of 200):\n";
    debugHeap();

    // Now request 150 bytes. 
    // First Fit would take the 500 block (first hole).
    // Best Fit should take the 200 block (smaller hole, tighter fit).
    
    setStrategy(AllocStrategy::BEST_FIT);
    
    cout << "Requesting 150 bytes (Should pick the 200-byte hole, NOT the 500-byte one)...\n";
    void* bestFitPtr = my_malloc(150);
    
    debugHeap(); // Verify which block was taken

    return 0;
}
