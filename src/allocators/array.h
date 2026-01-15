#pragma once

#include <cassert>
#include <stdio.h>
#include <emscripten.h>

#include "allocator.h"
#include "../fundamental_defs.h"

// needs to be here so it's accessible to the header-implemented fns that depend on it
TAllocator* GetDefaultAllocator();
// needs to be here so it's accessible to the header-implemented fns that depend on it
TAllocator* GetTempAllocator();

#define DYNAMIC_ARRAY_FIND_REMOVE_BODY(RemoveFn)    \
    bool bFoundItem = false;                        \
    for (s32 i = 0; i < ItemCount; i++)             \
    {                                               \
        if (Items[i] == Item) {                     \
            RemoveFn(i);                            \
            if (!bRemoveAll) {                      \
                return true;                        \
            }                                       \
            bFoundItem = true;                      \
        }                                           \
    }                                               \
    return bFoundItem;

#define ARRAY_FIND_BODY                             \
    for (s32 i = 0; i < ItemCount; ++i)             \
    {                                               \
        if (Items[i] == Item)                       \
        {                                           \
            return i;                               \
        }                                           \
    }                                               \
    return -1;

// base type for arrays. can be used as allocator-less slice type.
template<typename T>
class TArrayBase
{
public:

    TArrayBase() {}

    TArrayBase(T* InData, s32 InCount) : Items(InData), ItemCount(InCount)
    {
        assert(InCount >= 0);
    }

    // index into the array
    inline T& operator [](s32 i)
    {
        assert(i < ItemCount);
        return Items[i];
    }

    // index into the array
    inline const T& operator [](s32 i) const
    {
        assert(i < ItemCount);
        return Items[i];
    }

    // get a pointer to the beginning of the array data
    T* Data() { return Items; }

    // get a pointer to the beginning of the array data
    const T* Data() const { return Items; }

    // get the item count in the array
    inline s32 Count() const { return ItemCount; }

    // find an element in the array. returns -1 if not found
    inline s32 Find(const T&& Item) const
    {
        ARRAY_FIND_BODY
    }

    // find an element in the array. returns -1 if not found
    inline s32 Find(const T& Item) const
    {
        ARRAY_FIND_BODY
    }

    // returns true if the array contains the element
    bool Contains(const T&& Item) const
    {
        return Find(Item) != -1;
    }

    // returns true if the array contains the element
    bool Contains(const T& Item) const
    {
        return Find(Item) != -1;
    }

    // sort the array in-place
    // copied from jai module Sort.jai
    void QuickSort(s32 (*SortProc)(const T* A, const T* B))
    {
        QuickSortHelper(Data(), Count(), SortProc);
    }

    T* begin()
    {
        return Items;
    }

    T* end()
    {
        return Items + ItemCount;
    }

protected:

    // credit to the writer(s) of the jai (programming language) module Sort.jai
    // and whoever came up with quicksort
    static void QuickSortHelper(T* Array, s32 ArrayCount, s32 (*SortProc)(const T* A, const T* B))
    {
        if (ArrayCount < 2)
        {
            return;
        }

        T P = Array[ArrayCount/2];

        s32 i = 0;
        s32 j = ArrayCount - 1;

        while (true)
        {
            while (SortProc(&Array[i], &P) < 0) { i++; }
            while (SortProc(&P, &Array[j]) < 0) { j--; }

            if (i >= j)
            {
                break;
            }

            std::swap(Array[i], Array[j]);

            i++;
            j--;
        }

        QuickSortHelper(Array, i, SortProc);
        QuickSortHelper(Array + i, ArrayCount - i, SortProc);
    }

protected:

    T* Items;
    s32 ItemCount;
};

// manually-sized heap array. it's nice to have simpler functionality available rather than always
// using an array that will resize. also, if the array is supposed to be a specific size, just
// make sure it doesn't grow by choosing the right array type.
template<typename T>
class TArray : public TArrayBase<T>
{
public:

    TArray() {
        Allocator = nullptr;
        Items = nullptr;
        ItemCount = 0;
    }

    TArray(s32 AllocCount, TAllocator* InAllocator=nullptr)
    {
        Init(AllocCount, InAllocator);
    }

    ~TArray() {}

    // initialize the array with an optional specific allocator, otherwise later the default allocator will be applied
    inline void Init(s32 AllocCount, TAllocator* InAllocator=nullptr)
    {
        assert(Allocator == nullptr);
        assert(Items == nullptr);
        assert(ItemCount == 0);
        Allocator = InAllocator;
        SetCount(AllocCount);
    }

    // initialize with the temporary (per-frame) allocator
    inline void TempInit(s32 AllocCount)
    {
        Init(AllocCount, GetTempAllocator());
    }

    // set the number of items in this array
    inline void SetCount(s32 NewCount)
    {
        TryInitializeAllocator();
        const s32 OldSize = ItemCount * sizeof(T);
        const s32 NewSize = NewCount * sizeof(T);
        Allocator->Realloc((void**)&Items, OldSize, NewSize);
        ItemCount = NewCount;
    }

    // nullify the array and return its allocation
    void Free()
    {
        if (Allocator)
        {
            Allocator->Free((void**)&Items);
        }
        Allocator = nullptr;
        ItemCount = 0;
    }

    // create a new copy of another array
    void Copy(TArrayBase<T>& Other)
    {
        SetCount(Other.Count());
        memcpy(Data(), Other.Data(), Count() * sizeof(T));
    }

    inline void SetAllocator(TAllocator* InAllocator)
    {
        // todo: check something really bad isn't being done here
        Allocator = InAllocator;
    }

    // called automatically before any allocations are made or freed. if an allocator isn't explicitly set,
    // just assign the default allocator
    inline void TryInitializeAllocator()
    {
        if (Allocator == nullptr)
        {
            Allocator = GetDefaultAllocator();
        }
    }

    using TArrayBase<T>::Data;
    using TArrayBase<T>::Count;

protected:


protected:

    // the allocator for this array is stored at runtime because it makes for less annoying templates nicer to use.
    TAllocator* Allocator;
    using TArrayBase<T>::Items;
    using TArrayBase<T>::ItemCount;
};

// resize array. add and remove items, having it auto-handle reallocations. like any decent resize 
// array, optimizes both for size (keep allocation small) and minimizing number of allocations (allocation larger than number of items requires.)
template<typename T>
class TDynamicArray : public TArrayBase<T>
{
    friend TArray<T>;
public:

    TDynamicArray()
    {
        Allocator = nullptr;
        Items = nullptr;
        ItemCount = 0;
        ItemMax = 0;
    }

    TDynamicArray(s32 AllocMax, TAllocator* InAllocator=nullptr)
    {
        Init(AllocMax, InAllocator);
    }

    ~TDynamicArray() {}

    // initialize the array with an optional specific allocator, otherwise later the default allocator will be applied
    inline void Init(s32 AllocMax, TAllocator* InAllocator=nullptr)
    {
        assert(Allocator == nullptr);
        assert(Items == nullptr);
        assert(ItemCount == 0);
        assert(ItemMax == 0);
        Allocator = InAllocator;
        SetMax(AllocMax);
    }

    // initialize with the temporary (frame-lifetime) allocator
    inline void TempInit(s32 AllocMax)
    {
        Init(AllocMax, GetTempAllocator());
    }

    // the maximum number of elements the array can have before it needs a resize
    inline s32 Max() const
    {
        return ItemMax;
    }

    // nullify the array, giving its allocation back
    void Free()
    {
        if (Allocator)
        {
            Allocator->Free((void**)&Items);
        }
        Allocator = nullptr;
        ItemCount = 0;
        ItemMax = 0;
    }

    // set the maximum number of elements the array can have before needing to resize
    inline void SetMax(s32 NewMax)
    {
        TryInitializeAllocator();
        const s32 OldSize = ItemMax * sizeof(T);
        const s32 NewSize = NewMax * sizeof(T);
        Allocator->Realloc((void**)&Items, OldSize, NewSize);
        ItemMax = NewMax;
        if (ItemMax < ItemCount)
        {
            ItemCount = ItemMax;
        }
    }

    // set the number of elements in the array. forces the maximum number of values up if necessary.
    void SetCount(s32 NewCount)
    {
        if (NewCount > ItemMax)
        {
            TryInitializeAllocator();
            const s32 OldSize = ItemMax * sizeof(T);
            const s32 NewSizeFloor = (NewCount < 4 ? 4 : NewCount)  * sizeof(T);
            ItemMax = Allocator->TryGrow((void**)&Items, OldSize, NewSizeFloor, true) / sizeof(T);
        }
        ItemCount = NewCount;
    }

    // set all items (count not max) to zero
    inline void Zero()
    {
        if (ItemMax > 0)
        {
            const s32 Size = ItemMax * sizeof(T);
            memzero(Items, 0, Size);
        }
    }

    // put an item in the array
    inline void Push(const T& Item)
    {
        SetCount(ItemCount + 1);
        Items[ItemCount-1] = Item;
    }

    // put an item in the array
    inline void Push(const T&& Item)
    {
        SetCount(ItemCount + 1);
        Items[ItemCount-1] = Item;
    }

    // put a default-constructed item in the array and return a reference to it
    inline T& Push() {
        SetCount(ItemCount + 1);
        Items[ItemCount-1] = {};
        return Items[ItemCount-1];
    }

    inline bool PushUnique(const T& Item)
    {
        const s32 Index = TArrayBase<T>::Find(Item);
        if (Index == -1)
        {
            Push(Item);
            return true;
        }
        return false;
    }

    // inline bool PushUnique(const T&& Item)
    // {
    //     const s32 Index = Find(Item);
    //     if (Index == -1)
    //     {
    //         Push(Item);
    //         return true;
    //     }
    //     return false;
    // }

    // add every element in another array to this array
    inline void Append(const TDynamicArray<T>& OtherArray)
    {
        if (OtherArray.Count() > 0)
        {
            const s32 OldCount = ItemCount;
            SetCount(ItemCount + OtherArray.Count());
            memcpy(Items + OldCount, OtherArray.Data(), OtherArray.Count() * sizeof(T));
        }
    }

    // remove the element at this index, retaining the order of the other elements
    inline void RemoveAtOrdered(s32 i)
    {
        assert(i < (s32)ItemCount);
        for (s32 idx = i; idx < ItemCount-1; idx++)
        {
            Items[idx] = Items[idx+1];
        }
        ItemCount--;
    }

    // remove the element at this index, swapping the last element
    // with the former element
    inline void RemoveAtUnordered(s32 i)
    {
        assert(i < (s32)ItemCount);
        if (i < ItemCount-1)
        {
            Items[i] = Items[ItemCount-1];
        }
        ItemCount--;
    }

    // remove the element returning true for == with Item, retaining the order of the other elements
    inline bool RemoveOrdered(const T&& Item, bool bRemoveAll=false)
    {
        DYNAMIC_ARRAY_FIND_REMOVE_BODY(RemoveAtOrdered)
    }
    
    // remove the element returning true for == with Item, retaining the order of the other elements
    inline bool RemoveOrdered(const T& Item, bool bRemoveAll=false)
    {
        DYNAMIC_ARRAY_FIND_REMOVE_BODY(RemoveAtOrdered)
    }

    // remove the element returning true for == with Item, swapping the last element with the former element
    inline bool RemoveUnordered(const T&& Item, bool bRemoveAll=false)
    {
        DYNAMIC_ARRAY_FIND_REMOVE_BODY(RemoveAtUnordered)
    }

    // remove the element returning true for == with Item, swapping the last element with the former element
    inline bool RemoveUnordered(const T& Item, bool bRemoveAll=false)
    {
        DYNAMIC_ARRAY_FIND_REMOVE_BODY(RemoveAtUnordered)
    }

    inline void SetAllocator(TAllocator* InAllocator)
    {
        // todo: check something really bad isn't being done here
        Allocator = InAllocator;
    }

    void Shrink()
    {
        SetMax(ItemCount);
    }

    // called automatically before any allocations are made or freed. if an allocator isn't explicitly set,
    // just assign the default allocator
    inline void TryInitializeAllocator()
    {
        if (Allocator == nullptr)
        {
            Allocator = GetDefaultAllocator();
        }
    }

public:

    using TArrayBase<T>::Data;
    using TArrayBase<T>::Count;

protected:

    // the allocator for this array is stored at runtime because it makes for less annoying templates nicer to use.
    TAllocator* Allocator;
    using TArrayBase<T>::Items;
    using TArrayBase<T>::ItemCount;
    s32 ItemMax;

};