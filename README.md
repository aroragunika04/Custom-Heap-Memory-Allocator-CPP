# Custom Heap Memory Allocator in C++

*A miniature implementation of `malloc`/`free` to explore how memory management works internally.*

---

## Overview

This project implements a **custom dynamic memory allocator** in C++ that mimics the core behavior of `malloc` and `free`.
Instead of relying on the system allocator, memory is managed manually inside a **private heap** using block metadata, free lists, and allocation strategies.

The allocator demonstrates how operating systems and runtime libraries manage heap memory, including **block splitting, coalescing, and fragmentation handling**.

This project was built to deeply understand **low-level memory management and allocator design**.

---

## Key Features

* **Private Heap Implementation** (1 MB simulated heap)
* **Block Metadata Management** for tracking memory blocks
* **Doubly Linked Heap List** to maintain physical block order
* **Free List Optimization** for fast allocation search
* **Block Splitting** during allocation
* **Coalescing (Merging)** adjacent free blocks
* **Multiple Allocation Strategies**

  * First Fit
  * Best Fit
* **Heap Debugging Tool**

  * Visual block inspection
  * Fragmentation statistics
* **Error Detection**

  * Double free protection
  * Invalid pointer detection

---

## Memory Layout

Each memory block consists of **metadata followed by user-accessible memory**.

```text
| Block Metadata | User Memory |
```

Metadata stores:

* Block size
* Allocation status (free / used)
* Pointers to neighboring blocks
* Pointers used by the free list

Example heap structure:

```text
| meta | used memory | meta | free memory | meta | used memory |
```

---

## Allocation Flow

The allocator follows this process during memory allocation:

```text
my_malloc()
   ↓
Align requested size
   ↓
Search free list
   ↓
Choose block (First Fit / Best Fit)
   ↓
Split block if large enough
   ↓
Return pointer to user memory
```

Example:

Before allocation:

```text
|------------- FREE 500 -------------|
```

After `malloc(100)`:

```text
|100 USED|------ FREE 400 ------|
```

---

## Free Operation & Coalescing

When memory is freed, the allocator merges adjacent free blocks to reduce fragmentation.

```text
my_free()
   ↓
Mark block as free
   ↓
Merge with next free block
   ↓
Merge with previous free block
   ↓
Update free list
```

Example:

Before freeing middle block:

```text
|100 FREE|200 USED|300 FREE|
```

After free + coalescing:

```text
|--------------- FREE 600 ---------------|
```

---

## Fragmentation Measurement

The allocator tracks **external fragmentation** using:

```text
fragmentation = 1 - (largestFreeBlock / totalFreeSize)
```

This metric shows how scattered the free memory is and how much of it is usable for large allocations.

---

## Example Heap Debug Output

```text
--- Heap Statistics ---

Block 0: Addr=0x1000 Size=100 Status=USED
Block 1: Addr=0x1064 Size=200 Status=FREE
Block 2: Addr=0x112C Size=300 Status=FREE

--- Summary ---
Total Blocks: 3
Used Blocks: 1 (Size: 100)
Free Blocks: 2 (Size: 500)
Largest Free Block: 300
Fragmentation: 40%
```

---

**Block.h** – Metadata structure for memory blocks
**HeapAllocator.cpp** – Core allocation and deallocation logic
**HeapAllocator.h** – Public allocator interface
**main.cpp** – Demonstration and test scenarios

---
## Running the Project

Compile the program using `g++`:

```bash
g++ main.cpp allocator/HeapAllocator.cpp -o allocator
```

Run the executable:

```bash
./allocator
```
The demo runs several scenarios including:

- basic allocations
- block splitting
- free and coalescing
- error handling
- comparison of First Fit vs Best Fit
## Concepts Demonstrated

This project explores several **core systems programming concepts**:

* Heap memory management
* Pointer arithmetic
* Memory fragmentation
* Linked list data structures
* Dynamic memory allocation
* Block splitting and merging
* Allocation strategy trade-offs

---

## Example Test Scenarios

The test program demonstrates:

* Basic allocation
* Block splitting
* Free and coalescing
* Error detection
* Comparison between **First Fit** and **Best Fit**

---

## Learning Goals

This project was built to gain a deeper understanding of:

* How memory allocators work internally
* Why fragmentation occurs
* How runtime systems manage dynamic memory efficiently
* Trade-offs between different allocation strategies

---

## Author

Gunika
B.Tech Computer Science Engineering

---

