#include "super_allocator.h"
#include "../core.h"

// each heap is, at max granularity, composed of 8 byte particles
// [ | | | | | | | | ... | | | | ]
// chunks are made up of a runtime-known number of particles (here, n=2)
// |   |   |   |   | ... |   |   |
// ... there are 64 chunks per heap block
// ... and some variable number of heap blocks per heap
// it's hard to think about, but it works well
//
// in short, particles (8 bytes) -> chunks (n=? particles) -> heap blocks (64 chunks) -> heap (n=? blocks)

namespace
{
    enum class EAllocBin : u8
    {
        Standalone,
        Small,
        Medium,
        Large
    };

    struct FAllocHeader
    {
        union Metadata {
            EAllocBin Bin;
            u8 BitPattern;
        };
        u8 Chunk;
        u16 ChunkCount;
        u16 HeapBlock;
        s16 Heap;
    };
}

void TSuperAllocator::Init(TAllocator& HeapAllocator, TAllocator& BooksAllocator, s32 InDefaultHeapSize, s32 MinAllocSize, s32 InParticlesPerChunk)
{
    assert(IsPow2(InDefaultHeapSize));
    assert(Heaps.Max() == 0);

    *this = {};

    MinAllocationSize = MinAllocSize;
    ParticlesPerChunk = InParticlesPerChunk;

    ParentAllocator = &BooksAllocator;
    Heaps.SetAllocator(&BooksAllocator); // heapallocator is for allocating heap datas, not the heaps array

    if (InDefaultHeapSize > 0)
    {
        DefaultHeapSize = InDefaultHeapSize;
    }
    else
    {
        assert(InDefaultHeapSize == 0);
        DefaultHeapSize = MinAllocSize * SUPER_ALLOCATOR_CHUNKS_PER_HEAP_BLOCK * 64;
    }
    
    AddHeap(DefaultHeapSize);
}

void *TSuperAllocator::Alloc(unsigned int Size)
{
    return nullptr;
}

void TSuperAllocator::Realloc(void **Data, unsigned int OldSize, unsigned int NewSize)
{
}

void TSuperAllocator::Free(void **Data)
{
}

void TSuperAllocator::AddHeap(s32 MinBytes)
{
    assert(ParticlesPerChunk > 0);
    assert(MinBytes > 0);

    FSuperAllocatorHeap& NewHeap = Heaps.Push();
    NewHeap.Particles.SetAllocator(ParentAllocator);
    const s32 AllocHeapBlockCount = MAX(DivCeil(MinBytes, ChunkSize()), 2);
    assert(Heaps.Count() <= S16_MAX);
    assert(AllocHeapBlockCount < U16_MAX);
    if (AllocHeapBlockCount < 8 && !bDidSmallHeapBlockCountWarning)
    {
        LOG_WARNING("min bytes % leads to only % heap blocks being allocated per super allocator heap. the number of heap blocks per heap should probably be somewhere between 8 and 128", MinBytes, AllocHeapBlockCount);
        bDidSmallHeapBlockCountWarning = true;
    }
    NewHeap.Particles.SetCount(AllocHeapBlockCount * SUPER_ALLOCATOR_CHUNKS_PER_HEAP_BLOCK * ParticlesPerChunk);
    GrowBooks();
}

void TSuperAllocator::GrowBooks()
{
    TDynamicArray<s8> NewTempBytes;
    NewTempBytes.TempInit(MAX(BooksAllocationSize*2, 1024));

    const bool bDoCopy = OldListCount > 0;
    if (bDoCopy)
    {
        assert(Books.LargestGapPerList != nullptr);

    }

}


/*
Super_Allocator {
    books pointers:

    ----------- largest_gap_array_ptr    
    |     ----- lists_of_gaps_array_ptr 
    |     |     flag_set_array_ptr ---------------------------------------
    |     |                                                               |
    |     ------------------------------------------                      |
    v                                              V                      v
    [gaps data | lists_of_gaps data | flags data | [lists_of_gaps_ptrs] | [flags_ptrs]
                  ^    ^   ^   ^                    |  |   |   |           |   |  |  |
                  |    |   |   |--------------------   |   |   |           (point into the flags data. omitted arrows because diagram would get more confusing)
                  |    |   |---------------------------|   |   |
                  |    |------------------------------------   |
                  |--------------------------------------------|

*/