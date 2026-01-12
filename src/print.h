#pragma once

#include <stdlib.h>

#include "fundamental_defs.h"
#include "allocators/allocator.h"
#include "math.h"
#include "string.h"

inline constexpr s32 MAX_LOG_CONTEXT_CHARS = 55;

// todo: int, float, and string formatting
// struct IntFormatInfo
// {
//     s32 Base = 10;
//     s32 MinimumDigits = 1;
// };

// class FormatInt
// {
// public:

//     FormatInt(u64 InValue, IntFormatInfo&& InInfo) : Value(InValue), Info(InInfo), bNegative(false) {}
//     FormatInt(s64 InValue, IntFormatInfo&& InInfo) : Value(abs(InValue)), Info(InInfo), bNegative(InValue < 0) {}

// public:

//     u64 Value;
//     IntFormatInfo Info;
//     bool bNegative;
// };

enum class EArgType
{
    U8,
    U16,
    U32,
    U64,
    S8,
    S16,
    S32,
    S64,
    Float32,
    Float64,
    Character,
    Pointer,
    CString,
    TString,
    TVector2
};

class FmtArg
{
public:

    FmtArg(u8 Value)                { Type = EArgType::U8; U8 = Value; }
    FmtArg(u16 Value)               { Type = EArgType::U16; U16 = Value; }
    FmtArg(u32 Value)               { Type = EArgType::U32; U32 = Value; }
    FmtArg(u64 Value)               { Type = EArgType::U64; U64 = Value; }
    FmtArg(s8 Value)                { Type = EArgType::S8; S8 = Value; }
    FmtArg(s16 Value)               { Type = EArgType::S16; S16 = Value; }
    FmtArg(s32 Value)               { Type = EArgType::S32; S32 = Value; }
    FmtArg(s64 Value)               { Type = EArgType::S64; S64 = Value; }
    FmtArg(float32 Value)           { Type = EArgType::Float32; Float32 = Value; }
    FmtArg(float64 Value)           { Type = EArgType::Float64; Float64 = Value; }
    FmtArg(void* Value)             { Type = EArgType::Pointer; Pointer = Value; }
    FmtArg(const s8 Value[])        { Type = EArgType::CString; CString = &Value[0]; }
    FmtArg(class TString* Value)    { Type = EArgType::TString; String = Value; }
    FmtArg(TVector2 Value)          { Type = EArgType::TVector2; Vector2 = Value; }

public:

    EArgType Type;
    union
    {
        u8 U8;
        u16 U16;
        u32 U32;
        u64 U64;
        s8 S8;
        s16 S16;
        s32 S32;
        s64 S64;
        float32 Float32;
        float64 Float64;
        s8 Character;
        void* Pointer;
        const s8* CString;
        TString* String;
        TVector2 Vector2;
    };
};

void InlineReverse(s8* Buffer, s32 First, s32 Last);

bool PrintInt(s8** Buffer, u64 Int, bool bNegative, s32* BufLen);

bool PrintFloat(s8** Buffer, float64 Float, s32* BufLen);

bool PrintCString(s8** Buffer, const s8* Str, s32* BufLen);

bool PrintTString(s8** Buffer, TString& Str, s32* BufLen);

bool PrintTVector2(s8** Buffer, TVector2 Vector, s32* BufLen);

TString SprintImpl(const s8* Fmt, TAllocator* Allocator, FmtArg Args[], int ArgCount);

const s8* TruncateWithEllipsis(const s8* InStr, s32 MaxChars);

inline bool PrintChar(s8** Buffer, u8 Char, s32* BufLen)
{
    if (*BufLen <= 0)
    {
        return false;
    }
    **Buffer = *(s8*)&Char;
    *Buffer += 1;
    *BufLen -= 1;
    return true;
}

// like sprintf but the format specifiers are all just '%' and the types
// are deduced via the template. allocates with the default allocator.
template <class... Ts>
inline TString StrPrint(const s8* Fmt, Ts&&... Inputs)
{
    constexpr size_t ARG_COUNT = sizeof...(Inputs);
    FmtArg Args[] = { FmtArg(static_cast<Ts&&>(Inputs))... };
    return SprintImpl(Fmt, GetDefaultAllocator(), Args, ARG_COUNT);
}

// like sprintf but the format specifiers are all just '%' and the types
// are deduced via the template. allocates with the temp allocator.
template <class... Ts>
inline TString TempPrint(const s8 *Fmt, Ts &&...Inputs)
{
    constexpr size_t ARG_COUNT = sizeof...(Inputs);
    FmtArg Args[] = { FmtArg(static_cast<Ts&&>(Inputs))... };
    return SprintImpl(Fmt, GetTempAllocator(), Args, ARG_COUNT);
}

template <class... Ts>
inline void LogError(const s8 *Fmt, const char* Context, Ts &&...Inputs)
{
    constexpr int STACK_STR_LEN = 2048;
    constexpr size_t ARG_COUNT = sizeof...(Inputs);

    FmtArg Args[] = { FmtArg(static_cast<Ts&&>(Inputs))... };
    const TString Str = SprintImpl(Fmt, GetTempAllocator(), Args, ARG_COUNT);

    char StackStr[STACK_STR_LEN];
    emscripten_get_callstack(EM_LOG_C_STACK, StackStr, STACK_STR_LEN);

    printf("[ERROR]   :: %s :: %s\n%s\n", TruncateWithEllipsis(Context, MAX_LOG_CONTEXT_CHARS), Str.CStr(), StackStr);
}

template <class... Ts>
inline void LogWarning(const s8 *Fmt, const char* Context, Ts &&...Inputs)
{
    constexpr size_t ARG_COUNT = sizeof...(Inputs);
    FmtArg Args[] = { FmtArg(static_cast<Ts&&>(Inputs))... };
    const TString Str = SprintImpl(Fmt, GetTempAllocator(), Args, ARG_COUNT);
    printf("[WARNING] :: %s :: %s\n", TruncateWithEllipsis(Context, MAX_LOG_CONTEXT_CHARS), Str.CStr());
}

template <class... Ts>
inline void Log(const s8 *Fmt, const char* Context, Ts &&...Inputs)
{
    constexpr size_t ARG_COUNT = sizeof...(Inputs);
    FmtArg Args[] = { FmtArg(static_cast<Ts&&>(Inputs))... };
    const TString Str = SprintImpl(Fmt, GetTempAllocator(), Args, ARG_COUNT);
    printf("[LOG]     :: %s :: %s\n", TruncateWithEllipsis(Context, MAX_LOG_CONTEXT_CHARS), Str.CStr());
}

#define LOG_ERROR(FormatStr, ...) LogError(FormatStr, __PRETTY_FUNCTION__, ##__VA_ARGS__)

#define LOG_WARNING(FormatStr, ...) LogWarning(FormatStr, __PRETTY_FUNCTION__, ##__VA_ARGS__)

#define LOG(FormatStr, ...) Log(FormatStr, __PRETTY_FUNCTION__, ##__VA_ARGS__)
