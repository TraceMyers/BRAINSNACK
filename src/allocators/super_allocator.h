#pragma once

#include "allocator.h"

// super allocator is super
// general purpose allocator ported from my home library

// todo: finish copying over from old stuff

template<unsigned int ChunkSize>
struct TSuperAllocatorOuterChunk {
    unsigned char InnerChunks[64][ChunkSize];
};


class TSuperAllocator : public TAllocator {

public:

    void Init(unsigned int AllocatedSize, TAllocator& InParentAllocator);

    virtual void* Alloc(unsigned int Size) override;
    
    virtual void Realloc(void** Data, unsigned int OldSize, unsigned int NewSize) override;

    virtual void Free(void** Data) override;

protected:

    unsigned int MinAllocSize;
    void** ChunkLists;

};