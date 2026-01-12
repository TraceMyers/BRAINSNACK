#pragma once

#include <cassert>

#include "allocator.h"

// bump / arena / linear allocator. essentially, an allocation works like this:
// 1. user requests 16 bytes.
// 2. store the current pointer into the array of bytes.
// 3. advance the allocator pointer by 16 bytes.
// 4. return the stored pointer.
// this is the least expensive possible form of heap allocation. just reset the allocator when
// all of the allocations are out of lifetime. the most powerful usage is as a frame allocator.
// allocate stuff in frame scopes using one of these. don't free these, just let them go out of scope. then,
// at the end of the frame, reset the pointer to the head, clearing the previous frame's allocations.
// some allocators of this type auto-resize, but that isn't necessary for this project.
class TBumpAllocator : public TAllocator {
public:

    void Init(unsigned int AllocatedSize, TAllocator& InParentAllocator);

    // get an allocation of bytes
    virtual void* Alloc(unsigned int Size) override;
    
    // reallocate an allocation of bytes
    virtual void Realloc(void** UserData, unsigned int OldSize, unsigned int NewSize) override;

    // does nothing (see explanation above class name)
    virtual void Free(void** UserData) override {};

    // reset the current 
    void Reset(bool bKeepAllocation);

    // point to a place in the allocator's byte array where the user intends to return, clearing
    // all allocations that came after this point.
    unsigned int GetMark() const { return Offset; }

    // reset the allocator to a previous save point
    void ResetToMark(unsigned int InMark) { Offset = InMark; }

protected:

    unsigned int Offset;
};
