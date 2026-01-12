#pragma once

#include <stdio.h>
#include <cassert>

#include "allocator.h"
#include "../print.h"

// the null allocator. assign if you want a debug message about having an invalid allocator field somewhere.
class TNoAllocator : public TAllocator {

public:

    virtual void* Alloc(unsigned int Size) override
    {
        LOG_ERROR("tried to allocate with no allocator");
        assert(false);
    }

    virtual void Realloc(void** Data, unsigned int OldSize, unsigned int NewSize) override
    {
        LOG_ERROR("tried to reallocate with no allocator");
        assert(false);
    }

    virtual void Free(void** Data) override {}

};