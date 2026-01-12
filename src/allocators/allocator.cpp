#include <cassert>

#include "allocator.h"
#include "../core.h"

u32 TAllocator::TryGrow(void **UserData, u32 OldSize, u32 NewSize, bool CeilToPowerOf2)
{
    if (OldSize >= NewSize)
    {
        return OldSize;
    }
    else if (CeilToPowerOf2)
    {
        const u32 NewPow2Size = CeilToNextPow2(NewSize);
        Realloc(UserData, OldSize, NewPow2Size);
        return NewPow2Size;
    }
    else
    {
        Realloc(UserData, OldSize, NewSize);
        return NewSize;
    }
}

void TAllocator::SetParent(TAllocator *InAllocator, bool bAssertSafety)
{
    // not necessarily something you want. you may want to set the parent to NoAllocator after assigning an allocation to this allocator
    if (bAssertSafety)
    {
        assert(ParentAllocator == nullptr);
        assert(Data == nullptr);
        assert(Allocated == 0);
    }
    ParentAllocator = InAllocator;
}
