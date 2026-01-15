#pragma once

#include <stdio.h>
#include <type_traits>
#include <limits>
#include <string.h>

// ---------------------------------------------------------------------------------------------------------------------

typedef long long           s64;
typedef int                 s32;
typedef short               s16;
typedef char                s8;

typedef unsigned long long  u64;
typedef unsigned int        u32;
typedef unsigned short      u16;
typedef unsigned char       u8;

typedef float               float32;
typedef double              float64;

// ---------------------------------------------------------------------------------------------------------------------

constexpr s64 S64_MIN           = std::numeric_limits<s64>::min();
constexpr s64 S64_MAX           = std::numeric_limits<s64>::max();
constexpr s32 S32_MIN           = std::numeric_limits<s32>::min();
constexpr s32 S32_MAX           = std::numeric_limits<s32>::max();
constexpr s16 S16_MIN           = std::numeric_limits<s16>::min();
constexpr s16 S16_MAX           = std::numeric_limits<s16>::max();
constexpr s8 S8_MIN             = std::numeric_limits<s8>::min();
constexpr s8 S8_MAX             = std::numeric_limits<s8>::max();

constexpr u64 U64_MIN           = 0;
constexpr u64 U64_MAX           = std::numeric_limits<u64>::max();
constexpr u32 U32_MIN           = 0;
constexpr u32 U32_MAX           = std::numeric_limits<u32>::max();
constexpr u16 U16_MIN           = 0;
constexpr u16 U16_MAX           = std::numeric_limits<u16>::max();
constexpr u8 U8_MIN             = 0;
constexpr u8 U8_MAX             = std::numeric_limits<u8>::max();

constexpr float32 FLOAT32_MAX   = std::numeric_limits<float32>::max();

// ---------------------------------------------------------------------------------------------------------------------

constexpr float32 PI            = 3.14159f;

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(x, a, b) ((x) < (a) ? (a) : ((x) > (b) ? (b) : (x)))
#define SQUARE(x) x * x

// *** boilerplating the Round()s to avoid writing complex templates. ***
// originally wrote these because they worked better for rounding extreme values in jai projects.
// they were copy-pasted here jic i want to use them. todo: check if necessary.

inline s32 Round(float32 InFloat)
{
    if (InFloat < S32_MIN) // underflow
    {
        return S32_MIN;
    }
    const s32 RoundedTowardZero = (s32)InFloat;
    if (InFloat > 0 && RoundedTowardZero < 0)  // overflow
    {
        return S32_MAX;
    }
    const float32 Diff = InFloat - (float32)RoundedTowardZero;
    if (Diff > 0.5f)
    {
        return RoundedTowardZero + 1;
    }
    else if (Diff < -0.5f)
    {
        return RoundedTowardZero - 1;
    }
    else
    {
        return RoundedTowardZero;
    }
}

inline s64 Round(float64 InFloat)
{
    if (InFloat < S64_MIN) // underflow
    {
        return S64_MIN;
    }
    const s64 RoundedTowardZero = (s64)InFloat;
    if (InFloat > 0 && RoundedTowardZero < 0)  // overflow
    {
        return S64_MAX;
    }
    const float64 Diff = InFloat - (float64)RoundedTowardZero;
    if (Diff > 0.5)
    {
        return RoundedTowardZero + 1;
    }
    else if (Diff < -0.5)
    {
        return RoundedTowardZero - 1;
    }
    else
    {
        return RoundedTowardZero;
    }
}

// ---------------------------------------------------------------------------------------------------------------------

constexpr u32 ONE_KB = 1024;
constexpr u32 ONE_MB = ONE_KB * 1024;
constexpr u32 ONE_GB = ONE_MB * 1024;

// object stuff feeling misplaced, but simply sidesteps include problems
// ---------------------------------------------------------------------------------------------------------------------

enum class EObjectType : u16 {
    None,
    Player,
    Camera,
    Environment,
    NPC,
    OneOffEffect,
    Teleporter
};

struct FObjectRef
{
    EObjectType Type    = EObjectType::None;
    s16 Index           = 0;
    s32 ID              = 0;

    bool operator == (const FObjectRef& Other) const
    {
        return memcmp(this, &Other, sizeof(FObjectRef)) == 0;
    }

    bool operator != (const FObjectRef& Other) const
    {
        return memcmp(this, &Other, sizeof(FObjectRef)) != 0;
    }
};

constexpr s32 INVALID_OBJECT_ID = 0;

// annoying stuff cpp makes you do to use your enum flags
// all credit to the coders whose work fed the AI golem
// ---------------------------------------------------------------------------------------------------------------------
template <typename Enum>
constexpr bool bEnableEnumFlags = false;

template <typename Enum>
constexpr std::enable_if_t<bEnableEnumFlags<Enum>, Enum>
operator |(Enum A, Enum B)
{
    using T = std::underlying_type_t<Enum>;
    return static_cast<Enum>(static_cast<T>(A) | static_cast<T>(B));
}

template <typename Enum>
constexpr std::enable_if_t<bEnableEnumFlags<Enum>, Enum>
operator &(Enum A, Enum B)
{
    using T = std::underlying_type_t<Enum>;
    return static_cast<Enum>(static_cast<T>(A) & static_cast<T>(B));
}

template <typename Enum>
constexpr std::enable_if_t<bEnableEnumFlags<Enum>, Enum>
operator ~(Enum A)
{
    using T = std::underlying_type_t<Enum>;
    return static_cast<Enum>(~static_cast<T>(A));
}

template <typename Enum>
constexpr std::enable_if_t<bEnableEnumFlags<Enum>, Enum>
operator |=(Enum& A, Enum B)
{
    return A = A | B;
}

template <typename Enum>
constexpr std::enable_if_t<bEnableEnumFlags<Enum>, Enum>
operator &=(Enum& A, Enum B)
{
    return A = A & B;
}

// ---------------------------------------------------------------------------------------------------------------------

// this hash implementation was copied from the jai module Hash.jai. I would just use that code, but
// we're in cpp land here. all credit to the writers of that module as well as the people who developed
// the fnv1a hash. here is the credit/comment from Hash.jai:

// FNV-1a hash: http://www.isthe.com/chongo/tech/comp/fnv/index.html
// It provides pretty good distribution and performance.  Not as good
// as Knuth for single values, but chains better for repeated hashing.
// It should be xor folded to the desired range rather than shifted.

inline constexpr u32 HASH_SEED = 5381;

inline u32 GetHash(s8* Bytes, s32 ByteCount, u32 Seed=HASH_SEED)
{
    constexpr u64 FNV_64_PRIME = 0x100000001b3;
    u64 Hash = Seed;
    for (s32 i = 0; i < ByteCount; i++)
    {
        const u64 Byte64 = *(u8*)&Bytes[i];
        const u64 Convolution = Hash ^ Byte64;
        Hash = Convolution * FNV_64_PRIME;
    }
    return (u32)Hash;
}

// ---------------------------------------------------------------------------------------------------------------------

// yet again jai modules to the rescue, keeping it maximum simple and high quality. PCG.jai
constexpr u64 PCG_DEFAULT_MULTIPLIER_64 = 6364136223846793005;
constexpr u64 PCG_DEFAULT_INCREMENT_64 = 1442695040888963407;
// made this thread local because you would want that in a project that does intentional multithreading. might as well.
// (why: otherwise, you get race conditions to read and modify the state)
inline thread_local u64 RandomState = PCG_DEFAULT_INCREMENT_64;

inline u32 Random()
{
    u64 OldState = RandomState;
    
    // Advance internal state
    RandomState = RandomState * PCG_DEFAULT_MULTIPLIER_64 + PCG_DEFAULT_INCREMENT_64;

    // Calculate output function (XSH RR), uses old state for max ILP
    const u32 XorShifted = (u32)(((OldState >> 18) ^ OldState) >> 27);
    const u32 Rot = (u32)(OldState >> 59);
    return (XorShifted >> Rot) | (XorShifted << ((~Rot+1) & 31));
}

inline float32 RandomZeroToOne()
{
    return (float32)((float64)Random() / U32_MAX);
}

// poor results near min and max values of float32
inline float32 RandomFloat(float Min, float Max)
{
    return CLAMP(Min + RandomZeroToOne() * (Max - Min), Min, Max);
}

// poor results anywhere near U32_MAX
inline s32 RandomInt(s32 Min, s32 Max)
{
    // why + 1.0f to FairMax:
    // consider the call RandomInt(0, 2); with an expected return value of 0, 1, or 2.
    // if we call RandomFloat(0, 2), then we get a uniform sample in [0, 2]. rounding that gets...
    // [0.0, 0.5] -> 0 (25% probability)
    // [0.5, 1.5] -> 1 (50% probability)
    // [1.5, 2.0] -> 2 (25% probability)
    // however...
    // RandomFloat(0, 3) -> uniform sample in [0, 3]. truncating and clamping gets...
    // [0.0, 1.0] -> 0 (33%)
    // [1.0, 2.0] -> 1 (33%)
    // [2.0, 3.0] -> 2 (33%)
    const float32 FairMin = (float32)Min;
    const float32 FairMax = ((float32)Max) + 1.0f;
    return CLAMP((s32)(RandomFloat(FairMin, FairMax)), Min, Max);
}

inline void PrintBits(u64 Value)
{
    for (int i = 63; i >= 0; i--)
    {
        if (Value & ((u64)1 << i))
        {
            putchar('1');
        }
        else
        {
            putchar('0');
        }
    }
    putchar('\n');
}