#pragma once

#include "../fundamental_defs.h"
#include "array.h"

// an array of bits. index to a specific bit, set, and get it as a boolean
class TBitArray
{
    friend class PoolHelper;
public:

    // set the number of bits
    void SetCapacity(s32 InBitCount, bool bTryShrink=false);

    // set the value of the bit at this bit indec
    void SetBit(s32 Bit, bool bValue);

    // check if the bit at the given bit index is 1
    bool IsBitSet(s32 Bit) const;

    // frees and nullifies the underlying allocation
    void Reset();

    // get the index of the first bit with the value 1
    s32 FindFirstSetBit() const;
    
    // get the index of the last bit with the value 1
    s32 FindLastSetBit();

    // get the index of the first bit with the value 0
    s32 FindFirstUnsetBit() const;

    // get the number of bits that can be get and set by index
    inline s32 GetBitCount() const { return BitCount; }

protected:

    TDynamicArray<u64> BitBlocks;
    s32 BitCount;

};