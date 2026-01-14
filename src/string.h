#pragma once

#include <cassert>

#include <emscripten.h>

#include "fundamental_defs.h"
#include "allocators/array.h"

inline bool IsUpperAlpha(const s8 C)
{
    return C >= 'A' && C <= 'Z';
}

inline bool IsLowerAlpha(const s8 C)
{
    return C >= 'a' && C <= 'z';
}

inline bool IsNumeric(const s8 C)
{
    return C >= '0' && C <= '9';
}

inline bool IsAlphaNumeric(const s8 C)
{
    return IsLowerAlpha(C) || IsUpperAlpha(C) || IsNumeric(C);
}

inline s8 ToLower(const s8 C)
{
    return IsUpperAlpha(C) ? C - 'A' + 'a' : C;
}

inline s8 ToUpper(const s8 C)
{
    return IsLowerAlpha(C) ? C - 'a' + 'A' : C;
}


// string implementation that tries to minimize copying behavior, and trusts the user to make
// potentially unsafe manipulations of the fields directly.
class TString
{
public:

    TString() : Count(0), Data(nullptr), Allocator(nullptr) {}

    TString(const s8* InStr)
    {
        *this = WrapStr(InStr);
    }

    // avoid complicated auto copy and move behaviors (like pushing to an array) by *not* freeing on destruction.
    // the simplest way to handle strings is to temp allocate whenever possible and pay attention when not doing that.
    // the fears we have about leaks cause us to make hair-pulling overcomplicated stuff with a lot of redundant copies.
    ~TString() {};

    // make a new copy of a c string's data as a TString
    static TString Copy(const s8* InStr, TAllocator* Allocator);

    // convenience wrapper for Copy. allocate using the temp allocator.
    static TString Temp(const s8* InStr);

    // convenience wrapper for Copy. allocate using the default allocator.
    static TString New(const s8* InStr);

    // point directly at the string without copying. only use if you know what you're doing.
    static TString WrapStr(const s8* InStr);

    // creates a wrapper string that points to the given data, as well as contain its byte count.
    // mental model = a string is an arbitrary string of bytes, not just text.
    // note that if you have a type that contains a pointer to data, such as a TArray, the thing
    // being wrapped is the TArray, not its data
    template<typename T>
    inline static TString Wrap(T& Item)
    {
        TString String;
        String.Data = (s8*)&Item;
        String.Count = sizeof(T);
        return String;
    }

    template<typename T>
    inline static TString WrapData(const T* Array, s32 Count)
    {
        TString String;
        String.Data = (s8*)Array;
        String.Count = sizeof(T) * Count;
        return String;
    }

    TString Duplicate();

    TString Copy(TAllocator* Allocator=nullptr);

    TString Substring(s32 Begin=0, s32 Count=-1);

    void ToLowerInline();

    void ToUpperInline();

    TString ToLowerCopy(TAllocator* Allocator=nullptr);

    TString ToUpperCopy(TAllocator* Allocator=nullptr);

    inline void Advance(s32 AdvanceCount)
    {
        assert(AdvanceCount <= Count);
        Data += AdvanceCount;
        Count -= AdvanceCount;
    }

    bool operator ==(const s8* Other) const;

    bool operator ==(const TString& Other) const;

    bool Contains(const s8* Other) const;

    s32 Find(const s8* Other) const;

    bool Split(const s8 Char, TString& Left, TString& Right);

    // WARNING: the output here is only guaranteed to be safe to use for the lifetime
    // of any temp allocations made within the scope of the call... but that is what
    // this should be used for 100% of the time, so...
    // if'n you want a permanent one, make a permanent copy
    const s8* CStr() const;

    void Release();

    s8& operator [](s32 i)
    {
        assert(i >= 0 && i < Count);
        return Data[i];
    }

    const s8& operator [](s32 i) const
    {
        assert(i >= 0 && i < Count);
        return Data[i];
    }

    inline TString TrimLeft()
    {
        TString T = Duplicate();
        while (T.Count > 0 && *T.Data == ' ')
        {
            T.Data++;
            T.Count--;
        }
        return T;
    }

    inline TString TrimRight()
    {
        TString T = Duplicate();
        while (T.Count > 0 && T.Data[T.Count-1] == ' ')
        {
            T.Count--;
        }
        return T;
    }

    inline TString Trim()
    {
        TString T = Duplicate();
        while (T.Count > 0 && *T.Data == ' ')
        {
            T.Data++;
            T.Count--;
        }       
        while (T.Count > 0 && T.Data[T.Count-1] == ' ')
        {
            T.Count--;
        }
        return T;
    }

    

public:

    s64 Count;
    s8* Data;
    TAllocator* Allocator;
};
