#pragma once

#include "allocator.h"

// an allocator that essentially wraps malloc and free, but also provides core allocator functions.
class TCStdAllocator : public TAllocator {

public:

    virtual void* Alloc(unsigned int Size) override;

    virtual void Realloc(void** UserData, unsigned int OldSize, unsigned int NewSize) override;

    virtual void Free(void** UserData) override;

};