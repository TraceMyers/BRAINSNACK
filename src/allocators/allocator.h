#pragma once

// the base allocator interface for the project. using virtual functions and inheritance allow for pretty ideal
// generic code for use in array and other data structure types. the preferred style for this project is to
// have allocators stored at runtime on the things that use them, except in contexts where the allocator is implicit.
class TAllocator {

public:

    // interface function. allocate bytes
    virtual void* Alloc(unsigned int Size) = 0;

    // interface function. reallocate bytes
    virtual void Realloc(void** UserData, unsigned int OldSize, unsigned int NewSize) = 0;

    // interface function. free an allocation
    virtual void Free(void** UserData) = 0;

    // grow the allocation if the NewSize is larger than the OldSize. if CeilToPowerOf2, NewSize will be ceiled to a power of 2
    unsigned int TryGrow(void** UserData, unsigned int OldSize, unsigned int NewSize, bool CeilToPowerOf2);

    // allocators have parent allocators. this makes for convenient tiered usage patterns like:
    // - runtime allocator allocates a big block of memory for the program's runtime, from the system (no parent) this
    // allocation is then treated as the program's runtime heap.
    // - a world reserves a heap from the runtime heap by settings its allocator to be the runtime allocator.
    // - a pool reserves an allocation from the world heap by setting its allocator to be the world allocator.
    // that's really the whole idea. can go more layers deep.
    void SetParent(TAllocator* InAllocator, bool bAssertSafety);

protected:

    TAllocator* ParentAllocator;
    void* Data;
    unsigned int Allocated;

};
