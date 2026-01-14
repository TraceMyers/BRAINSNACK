#pragma once

#include "allocator.h"
#include "../fundamental_defs.h"
#include "array.h"

// super allocator is super
// general purpose allocator ported from my home library
// maybe won't actually finish bringing this over. maybe cstd lib allocator is fine.

inline constexpr s32 SUPER_ALLOCATOR_CHUNKS_PER_HEAP_BLOCK = 64;
inline constexpr s32 SUPER_ALLOCATOR_DEFAULT_MIN_ALLOCATION_SIZE = 16;

struct FSuperAllocatorHeap
{
    static constexpr s32 PARTICLE_SIZE=8;
    TArray<u64> Particles;
};

struct FSuperAllocatorBooks
{
    u8* LargestGapPerList;
    TArrayBase<u8*> GapListOfLists;
    TArrayBase<u64> FlagListOfLists;
};

class TSuperAllocator : public TAllocator {
public:

    // MinAllocSize must be a power of 2
    void Init(TAllocator& HeapAllocator, TAllocator& BooksAllocator, s32 InDefaultHeapSize=0, s32 MinAllocSize=SUPER_ALLOCATOR_DEFAULT_MIN_ALLOCATION_SIZE, s32 ParticlesPerChunk=2);

    virtual void* Alloc(unsigned int Size) override;
    
    virtual void Realloc(void** Data, unsigned int OldSize, unsigned int NewSize) override;

    virtual void Free(void** Data) override;

    inline s32 ChunkSize() { return FSuperAllocatorHeap::PARTICLE_SIZE * ParticlesPerChunk; }

protected:

    void AddHeap(s32 MinBytes);

    void GrowBooks();

protected:

    TDynamicArray<FSuperAllocatorHeap> Heaps;
    FSuperAllocatorBooks Books; 
    s32 MinAllocationSize = SUPER_ALLOCATOR_DEFAULT_MIN_ALLOCATION_SIZE;
    s32 DefaultHeapSize;
    s32 ParticlesPerChunk=2;
    s32 BooksAllocationSize;
    s32 DefaultOuterChunkCountPerList = 32;
    s32 ShrinkToMinLists = 1;
    s32 DefaultReserveSize;
    s32 OldListCount;
    bool bDidSmallHeapBlockCountWarning = false;
};

