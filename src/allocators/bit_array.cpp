#include "bit_array.h"
#include "../math.h"

namespace
{
    // number of bits in the block size (the size of the integer in the backing array)
    // so, if the array allocates 64 bit ints, this will be 64
    static constexpr s32 BITS_PER_BLOCK = 64;
    // number to shift left by, or shift right by, to respectively multiply or divide by BITS_PER_BLOCK
    // i like using a hard-coded shift value in cases where an int divide would potentially
    // slow things down. it's unlikely the program would make it even up to an optimization
    // level -O1 without the optimizer doing this for you, but I like to make debug builds run decently.
    static constexpr s32 BITS_PER_BLOCK_MUL_DIV_SHIFT = 6;

    // the array int index and the bit index. this is how bits are indexed to.
    struct FBitIndexer
    {
        s32 Block;
        s32 Bit;
    };

    // convert a bit index to array index, bit index
    inline FBitIndexer ToIndexer(s32 Bit)
    {
        FBitIndexer BitIndexer;
        // shift right to divide, truncating bits below
        BitIndexer.Block = Bit >> BITS_PER_BLOCK_MUL_DIV_SHIFT;
        // cheap mod to get the bit index
        BitIndexer.Bit = Bit - (BitIndexer.Block << BITS_PER_BLOCK_MUL_DIV_SHIFT);
        return BitIndexer;
    }
}

void TBitArray::SetCapacity(s32 InBitCount, bool bTryShrink)
{
    BitCount = InBitCount;
    // divide and ceil because flooring would be truncating bits unless InBitCount is a power of 2
    const u64 BlockCount = DivCeil(InBitCount, BITS_PER_BLOCK);
    BitBlocks.SetCount(BlockCount);
    if (bTryShrink)
    {
        BitBlocks.Shrink();
    }
}

void TBitArray::SetBit(s32 Bit, bool bValue)
{
    assert(Bit < BitCount);
    const FBitIndexer Indexer = ToIndexer(Bit);
    const u64 ClearMask = ~((u64)1 << Indexer.Bit);
    const u64 SetMask = bValue  * ((u64)(1) << Indexer.Bit);
    BitBlocks[Indexer.Block] = (BitBlocks[Indexer.Block] & ClearMask) | SetMask;
}

bool TBitArray::IsBitSet(s32 Bit) const
{
    assert(Bit < BitCount);
    const FBitIndexer Indexer = ToIndexer(Bit);
    const u64 GetMask = (u64)1 << Indexer.Bit;
    return (BitBlocks[Indexer.Block] & GetMask) != 0;
}

void TBitArray::Reset()
{
    BitBlocks.Free();
    BitCount = 0;
}

s32 TBitArray::FindFirstSetBit() const
{
    s32 Bit = -1;
    for (int i = 0; i < BitBlocks.Count(); i++)
    {
        if (BitBlocks[i] != 0)
        {
            // the bit index = int array index * number of bits per int + count the number of zeroes, from most to least significant bits
            Bit = i * BITS_PER_BLOCK + __builtin_ctzll(BitBlocks[i]);
            if (Bit >= BitCount)
            {
                // shouldn't happen
                Bit = -1; 
            }
            break;
        }
    }
    return Bit;
}

s32 TBitArray::FindLastSetBit()
{
    s32 Bit = -1;
    for (int i = BitBlocks.Count() - 1; i >= 0; i--)
    {
        if (BitBlocks[i] != 0)
        {
            const s32 LeadingZeroCount = __builtin_clzll(BitBlocks[i]);
            const s32 LastSetBitIndex = BITS_PER_BLOCK - (LeadingZeroCount + 1);
            Bit = i * BITS_PER_BLOCK + LastSetBitIndex;
            if (Bit >= BitCount)
            {
                // out of the range of the number of bits the user specified. clearing and trying again.
                // might happen if capacity is set once then lowered slightly
                BitBlocks[i] &= ~(1 << LastSetBitIndex);
                i++;
                continue;
            }
            break;
        }
    }
    return Bit;
}

s32 TBitArray::FindFirstUnsetBit() const
{
    s32 Bit = -1;
    for (int i = 0; i < BitBlocks.Count(); i++)
    {
        if (BitBlocks[i] != U64_MAX)
        {
            const u64 FlippedBits = ~BitBlocks[i];
            Bit = i * BITS_PER_BLOCK + __builtin_ctzll(FlippedBits);
            if (Bit >= BitCount)
            {
                Bit = -1;
            }
            break;
        }
    }
    return Bit;
}
