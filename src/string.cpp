#include <string.h>
#include <cstdarg>
#include <cstdio>

#include "string.h"
#include "session.h"
#include "globals.h"
#include "math.h"

#define COPY_PASTA(StrData)                             \
    S.Data = (s8*)InAllocator->Alloc(S.Count + 1);      \
    S.Allocator = InAllocator;                          \
    if (S.Count > 0)                                    \
    {                                                   \
        memcpy(S.Data, StrData, S.Count);               \
    }                                                   \
    S.Data[S.Count] = '\0';                         

TString TString::Temp(const s8* InStr)
{
    return Copy(InStr, &Session.FrameAllocator);
}

TString TString::New(const s8 *InStr)
{
    return Copy(InStr, Session.DefaultAllocator);
}

TString TString::WrapStr(const s8 *InStr)
{
    TString S;
    S.Count = strlen(InStr);
    S.Data = (s8*)InStr; // may the const police never find me
    return S;
}

TString TString::Copy(const s8 *InStr, TAllocator *InAllocator)
{
    TString S;
    S.Count = strlen(InStr);
    COPY_PASTA(InStr)
    return S;
}

TString TString::Duplicate()
{
    TString S;
    S.Count = Count;
    S.Data = Data;
    // no allocator, because this isn't the master copy
    return S;
}

TString TString::Copy(TAllocator* InAllocator)
{
    if (InAllocator == nullptr)
    {
        InAllocator = Session.DefaultAllocator;
    }
    TString S;
    S.Count = Count;
    COPY_PASTA(Data)
    return S;
}

TString TString::Substring(s32 Begin, s32 OutCount)
{
    if (OutCount == -1)
    {
        OutCount = Count - Begin;
    }
    const s32 ExpectedCount = Begin + OutCount;
    assert(Count >= ExpectedCount);
    TString S;
    S.Count = OutCount;
    S.Data = Data + Begin;
    return S;
}

void TString::ToLowerInline()
{
    for (int i = 0; i < Count; i++)
    {
        Data[i] = ToLower(Data[i]);
    }
}

void TString::ToUpperInline()
{
    for (int i = 0; i < Count; i++)
    {
        Data[i] = ToUpper(Data[i]);
    }
}

TString TString::ToLowerCopy(TAllocator *InAllocator)
{
    TString S = Copy(InAllocator);
    S.ToLowerInline();
    return S;
}

TString TString::ToUpperCopy(TAllocator *InAllocator)
{
    TString S = Copy(InAllocator);
    S.ToUpperInline();
    return S;
}

bool TString::operator==(const s8 *Other) const
{
    for (int i = 0; i < Count; i++)
    {
        if (Data[i] != Other[i])
        {
            return false;
        }
        if (Other[i] == '\0')
        {
            return false;
        }
    }
    return true;
}

bool TString::operator==(const TString &Other) const
{
    if (Count != Other.Count)
    {
        return false;
    }
    return memcmp(Data, Other.Data, Count) == 0;
}

bool TString::Contains(const s8 *Other) const
{
    return Find(Other) >= 0;
}

s32 TString::Find(const s8 *Other) const
{
    s32 BeginMatch = -1;
    const s8* OtherPtr = Other;
    for (s32 i = 0; i < Count; i++)
    {
        if (Data[i] == *OtherPtr)
        {
            if (BeginMatch == -1)
            {
                BeginMatch = i;
            }
            OtherPtr++;
            if (*OtherPtr == '\0')
            {
                break;
            }
        }
        else 
        {
            OtherPtr = Other;
            BeginMatch = -1;
        }
    }
    return BeginMatch;
}

bool TString::Split(const s8 Char, TString &Left, TString &Right)
{
    for (s32 i = 0; i < Count; i++)
    {
        if (Data[i] == Char)
        {
            Left.Data = Data;
            Left.Count = i;
            if (i + 1 < Count)
            {
                Right.Data = Data + (i + 1);
                Right.Count = Count - (i + 1);
            }
            else
            {
                Right = {};
            }
            return true;
        }
    }
    return false;
}

const s8 *TString::CStr() const
{
    s8* TempData = (s8*)TempAlloc(Count + 1);
    if (Count > 0)
    {
        memcpy(TempData, Data, Count);
    }
    TempData[Count] = '\0';
    return TempData;
}

void TString::Release()
{
    // note that setting the allocator is optional, allowing for inline copies
    if (Allocator)
    {
        Allocator->Free((void**)&Data);
    }
    Count = 0;
    Data = nullptr;
    Allocator = nullptr;
}
