#pragma once

#include "../fundamental_defs.h"
#include "array.h"
#include "bit_array.h"

enum class EPoolResizeBehavior : u8
{
    Default,
    Allow,
    Disallow
};

// an allocator of uniformly-typed objects.
template <typename T>
class TPool
{
    friend class PoolHelper;
    // todo: delete this
    friend class TSession;
public:

    void SetCapacity(s32 Capacity);

    void LockAllocation();

    void Reset(void (*ReleaseItemProc)(T*)=nullptr);

    s32 RequestItem(void (*InitItemProc)(T*)=nullptr, EPoolResizeBehavior GrowBehavior=EPoolResizeBehavior::Default);

    void ReturnItem(s32 i, void (*ReleaseItemProc)(T*)=nullptr, EPoolResizeBehavior ShrinkBehavior=EPoolResizeBehavior::Default);

    inline T& operator[](s32 i)
    {
        assert(IsItemInUse(i));
        return Items[i];
    }

    inline const T& operator[](s32 i) const
    {
        assert(IsItemInUse(i));
        return Items[i];
    }

    inline bool IsItemInUse(s32 i) const { return InUse.IsBitSet(i); }

    inline s32 ItemIndex(const T& Item) const { const s32 Index = &Item - Items.Items; }

    inline s32 TopIndex() const { return Top; }

    inline s32 ItemCount() const { return Items.Count(); }

protected:

    TDynamicArray<T> Items;
    TBitArray InUse;
    s32 Top = -1;
    s32 ShrinkToMin = 0;
    bool bDefaultAllowGrow = true;
    bool bDefaultAllowShrink = true;
};

// template specifications have to be defined where they are declared...
// ---------------------------------------------------------------------------------------------------------------------

template <typename T>
inline void TPool<T>::SetCapacity(s32 Capacity)
{
    Items.SetCount(Capacity);
    InUse.SetCapacity(Capacity); 
}

// template specifications have to be defined in headers because c++ is messy.
// this is a hack that sidesteps include issues
class PoolHelper
{
public:
    static void LockPoolAllocation(TPool<u8>* Pool);
};

template <typename T>
inline void TPool<T>::LockAllocation()
{
    PoolHelper::LockPoolAllocation((TPool<u8>*)this);
}

template <typename T>
inline void TPool<T>::Reset(void (*ReleaseItemProc)(T*))
{
    if (Top >= 0 && ReleaseItemProc != nullptr)
    {
        for (int i = 0; i <= Top; i++)
        {
            if (InUse.IsBitSet(i))
            {
                ReleaseItemProc(&Items[i]);
            }
        }
    }
    Items.Free();
    InUse.Reset();
    Top = -1;
}

template <typename T>
inline s32 TPool<T>::RequestItem(void (*InitItemProc)(T*), EPoolResizeBehavior GrowBehavior)
{
    s32 i = InUse.FindFirstUnsetBit();
    if (i != -1)
    {
        InUse.SetBit(i, true);
        if (i > Top)
        {
            Top = i;
            Items.SetCount(MAX(Items.Count(), Top+1));
        }
        if (InitItemProc != nullptr)
        {
            InitItemProc(&Items[i]);
        }
        return i;
    }

    const bool bDoAllowGrow = (GrowBehavior == EPoolResizeBehavior::Default ? bDefaultAllowGrow : GrowBehavior == EPoolResizeBehavior::Allow);
    if (!bDoAllowGrow)
    {
        printf("pool failed to allocate. top %d\n", Top);
        return -1;
    }

    i = Items.Count();
    Top = Items.Count();

    constexpr s32 RESIZE_BUFFER = 8;
    const s32 NewItemCount = Items.Count() + RESIZE_BUFFER;
    Items.SetCount(NewItemCount);
    InUse.SetCapacity(NewItemCount);
    InUse.SetBit(i, true);
    if (InitItemProc != nullptr)
    {
        InitItemProc(&Items[i]);
    }

    return i;
}

template <typename T>
inline void TPool<T>::ReturnItem(s32 i, void (*ReleaseItemProc)(T*), EPoolResizeBehavior ShrinkBehavior)
{
    if (ReleaseItemProc != nullptr)
    {
        ReleaseItemProc(&Items[i]);
    }

    InUse.SetBit(i, false);

    if (i == Top)
    {
        Top = InUse.FindLastSetBit();
        const bool bDoAllowShrink = (ShrinkBehavior == EPoolResizeBehavior::Default ? bDefaultAllowShrink : ShrinkBehavior == EPoolResizeBehavior::Allow);

        if (bDoAllowShrink && Items.Count() > ShrinkToMin)
        {
            const s32 RequiredCount = MAX(Top + 1, ShrinkToMin);
            if (RequiredCount >= Items.Count())
            {
                return;
            }

            // div by 2 repeatedly until we find the threshold we're under
            s32 TestThreshold = Items.Count() >> 1;
            while (RequiredCount < TestThreshold)
            {
                TestThreshold >>= 1;
            }

            // only shrink by up to half the space we could shrink, to leave room for incoming adds
            TestThreshold <<= 1;
            if (TestThreshold >= Items.Count())
            {
                return;
            }

            const s32 NewCount = MAX(TestThreshold, ShrinkToMin);
            Items.SetCount(NewCount);

            if (NewCount == 0)
            {
                Items.SetMax(0);
            }
            else if (Items.Count() < 8)
            {
                Items.SetMax(8);
            }
            else if (Items.Count() < Items.Max() / 4)
            {
                const s32 NewMax = Items.Max() / 2;
                Items.SetMax(NewMax);
            }

            InUse.SetCapacity(true);
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
