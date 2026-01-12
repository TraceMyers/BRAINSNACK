#include "super_allocator.h"
#include "../core.h"

void TSuperAllocator::Init(u32 AllocatedSize, TAllocator &InParentAllocator)
{
}

void* TSuperAllocator::Alloc(u32 Size)
{
    return nullptr;
}

void TSuperAllocator::Realloc(void **Data, u32 OldSize, u32 NewSize)
{
}

void TSuperAllocator::Free(void **Data)
{
}
