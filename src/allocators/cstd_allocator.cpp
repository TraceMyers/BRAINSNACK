#include <stdlib.h>
#include <cassert>

#include "cstd_allocator.h"
#include "../core.h"

namespace
{
    constexpr u32 ALIGN = 8;
}

void *TCStdAllocator::Alloc(unsigned int Size)
{
    Size = CeilToMultipleOfPow2(Size, ALIGN);
    return malloc(Size);
}

void TCStdAllocator::Realloc(void **UserData, unsigned int OldSize, unsigned int NewSize)
{
    NewSize = CeilToMultipleOfPow2(NewSize, ALIGN);
    void* NewData = nullptr;
    if (NewSize > 0)
    {
        NewData = malloc(NewSize);
        assert(NewData != nullptr);
        u32 CopySize = MIN(OldSize, NewSize);
        if (CopySize > 0)
        {
            memcpy(NewData, *UserData, CopySize);
        }
    } 
    if (OldSize > 0)
    {
        free(*UserData);
    }
    *UserData = NewData;    
}

void TCStdAllocator::Free(void **UserData)
{
    free(*UserData);
    *UserData = nullptr;
}
