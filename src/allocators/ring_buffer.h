#pragma once

#include <cassert>
#include <stdio.h>

#include "array.h"
#include "math.h"

template<typename T>
class TRingBuffer
{
public:

    void Init(s32 AllocCount, TAllocator* InAllocator=nullptr)
    {
        Items.Init(AllocCount, InAllocator);
        Front = 0;
        Back = 0;
        ItemCount = 0;
    }

    s32 Count() const
    {
        return ItemCount;
    }

    void PushFront(const T& Item)
    {
        Items[Front] = Item;
        const s32 OldFront = Front;
        Front = Increment(Front);
        if (ItemCount == Items.Count())
        {
            assert(Back == OldFront);
            Back = Front;
        }
        else
        {
            assert(ItemCount < Items.Count());
            ItemCount++;
        }
    }

    void PushBack(const T& Item)
    {
        Items[Back] = Item;
        const s32 OldBack = Back;
        Back = Decrement(Back);
        if (ItemCount == Items.Count())
        {
            assert(Front == OldBack);
            Front = Back;
        }
        else
        {
            assert(ItemCount < Items.Count());
            ItemCount++;
        }
    }

    T PopFront()
    {
        assert(ItemCount > 0);
        Front = Decrement(Front);
        T Item = Items[Front];
        ItemCount--;
    }

    T PopBack()
    {
        assert(ItemCount > 0);
        Back = Increment(Back);
        T Item = Items[Back];
        ItemCount--;
    }

    T& PeekFront()
    {
        assert(ItemCount > 0);
        const s32 Idx = Decrement(Front);
        return Items[Idx];
    }

    T& PeekBack()
    {
        assert(ItemCount > 0);
        const s32 Idx = Increment(Back);
        return Items[Idx];
    }

    void DumpBackToFront(TDynamicArray<T>& OutItems) const
    {
        s32 Idx = Increment(Back);
        for (s32 DumpCounter = ItemCount; DumpCounter > 0; DumpCounter--)
        {
            OutItems.Push(Items[Idx]);
            Idx = Increment(Idx);
        }
    }

    void DumpFrontToBack(TDynamicArray<T>& OutItems) const
    {
        s32 Idx = Decrement(Front);
        for (s32 DumpCounter = ItemCount; DumpCounter > 0; DumpCounter--)
        {
            OutItems.Push(Items[Idx]);
            Idx = Decrement(Idx);
        }
    }

    s32 Increment(s32 Index) const
    {
        return IncrementWrap(Index, 0, Items.Count()-1);
    }

    s32 Decrement(s32 Index) const
    {
        return DecrementWrap(Index, 0, Items.Count()-1);
    }

protected:

    TArray<T> Items;
    s32 Front;
    s32 Back;
    s32 ItemCount;
};