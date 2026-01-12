#include "bump_allocator.h"

#include "../core.h"

namespace
{
    constexpr u32 BUMP_ALLOCATOR_ALIGN = 8;
}

void TBumpAllocator::Init(u32 AllocatedSize, TAllocator& InParentAllocator)
{
    assert(Allocated == 0);
    assert(Offset == 0);
    ParentAllocator = &InParentAllocator;
    ParentAllocator->Realloc(&Data, 0, AllocatedSize);
    assert((u64)Data % BUMP_ALLOCATOR_ALIGN == 0);
    Allocated = AllocatedSize;
}

void* TBumpAllocator::Alloc(u32 Size)
{
    if (Size == 0)
    {
        return nullptr;
    }
    const u32 AlignedSize = CeilToMultipleOfPow2(Size, BUMP_ALLOCATOR_ALIGN);

    const u32 OldOffset = Offset;
    Offset += AlignedSize;
    // all bump allocators in the game get their allocation at init and don't realloc
    assert(Allocated >= Offset);

    void* RetPtr = (void*)((u8*)Data + OldOffset);
    return RetPtr;
}

void TBumpAllocator::Realloc(void **UserData, u32 OldSize, u32 NewSize)
{
    if (NewSize <= OldSize)
    {
        return;
    }
    void* NewData = Alloc(NewSize);
    const u32 CopySize = MIN(OldSize, NewSize);
    if (CopySize > 0)
    {
        memcpy(NewData, *UserData, CopySize);
    }
    *UserData = NewData;
}

void TBumpAllocator::Reset(bool bKeepAllocation)
{
    Offset = 0;
    if (!bKeepAllocation)
    {
        ParentAllocator->Realloc(&Data, Allocated, 0);
        Allocated = 0;
    }
    else if (Allocated > 0)
    {
        memset(Data, 0, Allocated);
    }
}
