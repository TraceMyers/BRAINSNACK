#pragma once

#include <cmath>

#include "fundamental_defs.h"

class TVector2
{
public:

    TVector2() : X(0), Y(0) {}
    TVector2(float InX, float InY) : X(InX), Y(InY) {}

    inline TVector2 operator + (TVector2 OtherVec) const
    {
        return TVector2(X + OtherVec.X, Y + OtherVec.Y);
    }

    inline TVector2 operator - (TVector2 OtherVec) const
    {
        return TVector2(X - OtherVec.X, Y - OtherVec.Y);
    }

    inline TVector2 operator * (TVector2 OtherVec) const
    {
        return TVector2(X * OtherVec.X, Y * OtherVec.Y);
    }

    inline TVector2 operator / (TVector2 OtherVec) const
    {
        return TVector2(X / OtherVec.X, Y / OtherVec.Y);
    }

    inline TVector2 operator + (float32 Scalar) const
    {
        return TVector2(X + Scalar, Y + Scalar);
    }

    inline TVector2 operator - (float32 Scalar) const
    {
        return TVector2(X - Scalar, Y - Scalar);
    }

    inline TVector2 operator * (float32 Scalar) const
    {
        return TVector2(X * Scalar, Y * Scalar);
    }

    inline TVector2 operator / (float32 Scalar) const
    {
        const float32 InvScalar = 1.0f / Scalar;
        return TVector2(X * InvScalar, Y * InvScalar);
    }

    inline TVector2& operator += (TVector2 OtherVec)
    {
        X += OtherVec.X;
        Y += OtherVec.Y;
        return *this;
    }

    inline TVector2& operator -= (TVector2 OtherVec)
    {
        X += OtherVec.X;
        Y += OtherVec.Y;
        return *this;
    }

    inline TVector2& operator *= (TVector2 OtherVec)
    {
        X *= OtherVec.X;
        Y *= OtherVec.Y;
        return *this;
    }

    inline TVector2& operator /= (TVector2 OtherVec)
    {
        X /= OtherVec.X;
        Y /= OtherVec.Y;
        return *this;
    }

    inline TVector2& operator += (float32 Scalar)
    {
        X += Scalar;
        Y += Scalar;
        return *this;
    }

    inline TVector2& operator -= (float32 Scalar)
    {
        X -= Scalar;
        Y -= Scalar;
        return *this;
    }

    inline TVector2& operator *= (float32 Scalar)
    {
        X *= Scalar;
        Y *= Scalar;
        return *this;
    }

    inline TVector2& operator /= (float32 Scalar)
    {
        const float32 InvScalar = 1.0f / Scalar;
        X *= InvScalar;
        Y *= InvScalar;
        return *this;
    }

    inline float32 MagnitudeSq() const
    {
        return X * X + Y * Y;
    }

    inline float32 Magnitude() const
    {
        return sqrtf(X * X + Y * Y);
    }

    inline TVector2 Normal(float MinMagSq=3e-7f)
    {
        const float32 MagSq = MagnitudeSq();
        return MagSq < MinMagSq ? TVector2(0, 0) : *this / sqrtf(MagSq);
    }

    inline void Normalize(float MinMagSq=3e-7f)
    {
        *this = Normal(MinMagSq);
    }

    inline float32 Distance(TVector2 OtherVec)
    {
        return (*this - OtherVec).Magnitude();
    }

    inline float32 DistanceSq(TVector2 OtherVec)
    {
        return (*this - OtherVec).MagnitudeSq();
    }

public:

    float X;
    float Y;
};

class TVector2i
{
public:

    TVector2i() : X(0), Y(0) {}
    TVector2i(s32 InX, s32 InY) : X(InX), Y(InY) {}

    inline TVector2i operator + (TVector2i OtherVec) const
    {
        return TVector2i(X + OtherVec.X, Y + OtherVec.Y);
    }

    inline TVector2i operator - (TVector2i OtherVec) const
    {
        return TVector2i(X - OtherVec.X, Y - OtherVec.Y);
    }

    inline TVector2i operator * (TVector2i OtherVec) const
    {
        return TVector2i(X * OtherVec.X, Y * OtherVec.Y);
    }

    inline TVector2i operator / (TVector2i OtherVec) const
    {
        return TVector2i(X / OtherVec.X, Y / OtherVec.Y);
    }

    inline TVector2i operator + (s32 Scalar) const
    {
        return TVector2i(X + Scalar, Y + Scalar);
    }

    inline TVector2i operator - (s32 Scalar) const
    {
        return TVector2i(X - Scalar, Y - Scalar);
    }

    inline TVector2i operator * (s32 Scalar) const
    {
        return TVector2i(X * Scalar, Y * Scalar);
    }

    inline TVector2i operator / (s32 Scalar) const
    {
        return TVector2i(X / Scalar, Y / Scalar);
    }

    inline TVector2i& operator += (TVector2i OtherVec)
    {
        X += OtherVec.X;
        Y += OtherVec.Y;
        return *this;
    }

    inline TVector2i& operator -= (TVector2i OtherVec)
    {
        X += OtherVec.X;
        Y += OtherVec.Y;
        return *this;
    }

    inline TVector2i& operator *= (TVector2i OtherVec)
    {
        X *= OtherVec.X;
        Y *= OtherVec.Y;
        return *this;
    }

    inline TVector2i& operator /= (TVector2i OtherVec)
    {
        X /= OtherVec.X;
        Y /= OtherVec.Y;
        return *this;
    }

    inline TVector2i& operator += (s32 Scalar)
    {
        X += Scalar;
        Y += Scalar;
        return *this;
    }

    inline TVector2i& operator -= (s32 Scalar)
    {
        X -= Scalar;
        Y -= Scalar;
        return *this;
    }

    inline TVector2i& operator *= (s32 Scalar)
    {
        X *= Scalar;
        Y *= Scalar;
        return *this;
    }

    inline TVector2i& operator /= (s32 Scalar)
    {
        X /= Scalar;
        Y /= Scalar;
        return *this;
    }

public:

    s32 X;
    s32 Y;
};

struct TVector3
{
    TVector3() : X(0), Y(0), Z(0) {}
    TVector3(float32 InX, float32 InY, float32 InZ) : X(InX), Y(InY), Z(InZ) {}
    inline TVector3 operator * (float32 Scalar)
    {
        return TVector3(X * Scalar, Y * Scalar, Z * Scalar);
    }
    float32 X;
    float32 Y;
    float32 Z;

    inline void SetComponent(s32 i, float32 Value)
    {
        float* Self = (float*)this;
        Self[i] = Value;
    }

    TVector2& AsVector2()
    {
        return *(TVector2*)this;
    }
};

class TMatrix
{
public:
    void SetYRotation(const float32 Radians)
    {
        Components[0][0] = cos(Radians);
        Components[0][1] = 0;
        Components[0][2] = -sin(Radians);
        Components[1][0] = 0;
        Components[1][1] = 1;
        Components[1][2] = 0;
        Components[2][0] = sin(Radians);
        Components[2][1] = 0;
        Components[2][2] = cos(Radians);
    }

    TVector3 Mul(const TVector3& Vec)
    {
        return TVector3(
            Vec.X * Components[0][0] + Vec.Y * Components[0][1] + Vec.Z * Components[0][2],
            Vec.X * Components[1][0] + Vec.Y * Components[1][1] + Vec.Z * Components[1][2],
            Vec.X * Components[2][0] + Vec.Y * Components[2][1] + Vec.Z * Components[2][2]
        );
    }

    float Components[3][3];
};

// not using ctors because declaration order stuff overcomplicates things.
inline TVector2 ToVector2(const TVector2i& Vec)
{
    return TVector2((float)Vec.X, (float)Vec.Y);
}

inline TVector2i ToVector2i(const TVector2& Vec)
{
    return TVector2i((s32)Vec.X, (s32)Vec.Y);
}

enum class EDirection : u8
{
    None        = 0,
    Up          = 1 << 0,
    Right       = 1 << 1,
    Down        = 1 << 2,
    Left        = 1 << 3
};

inline EDirection RandomDirection()
{
    const s32 Draw = RandomInt(0, 3);
    switch (Draw)
    {
    case 0:
        return EDirection::Up;
    case 1:
        return EDirection::Right;
    case 2:
        return EDirection::Down;
    case 3:
        return EDirection::Left;
    default:
        return EDirection::None;
    }
}

// enable bitwise operators
template<>
inline constexpr bool bEnableEnumFlags<EDirection> = true;

inline TVector2 ToVector2(EDirection Direction)
{
    // todo: check if the branchless approach is worth it? or, I dunno, it's fine even if it doesn't add anything.
    TVector2 Dir(0, 0);
    const bool bMovingUp    = (Direction & EDirection::Up) != EDirection::None;
    const bool bMovingRight = (Direction & EDirection::Right) != EDirection::None;
    const bool bMovingDown  = (Direction & EDirection::Down) != EDirection::None;
    const bool bMovingLeft  = (Direction & EDirection::Left) != EDirection::None;
    Dir += TVector2( 0, -1) * (float)bMovingUp;
    Dir += TVector2( 1,  0) * (float)bMovingRight;
    Dir += TVector2( 0,  1) * (float)bMovingDown;
    Dir += TVector2(-1,  0) * (float)bMovingLeft;
    return Dir.Normal();
}

// doesn't need to be normalized
inline EDirection To8WayDirection(TVector2 Vector)
{
    constexpr float COS_PI_OVER_6 = 0.8660254f;
    constexpr float SIN_PI_OVER_6 = 0.5f;
    constexpr float COS_PI_OVER_3 = 0.5f;
    constexpr float SIN_PI_OVER_3 = 0.8660254f;

    if (Vector.Y == 0)
    {
        if (Vector.X == 0)
        {
            return EDirection::None;
        }
        else
        {
            return Vector.X > 0 ? EDirection::Right : EDirection::Left;
        }
    }
    else if (Vector.X == 0)
    {
        return Vector.Y > 0 ? EDirection::Down : EDirection::Up;
    }

    const float32 AbsXOverY = fabs(Vector.X / Vector.Y);

    if (AbsXOverY > (COS_PI_OVER_6 / SIN_PI_OVER_6))
    {
        return Vector.X > 0 ? EDirection::Right : EDirection::Left;
    }
    else if (AbsXOverY > (COS_PI_OVER_3 / SIN_PI_OVER_3))
    {
        if (Vector.X > 0)
        {
            return Vector.Y > 0 ? (EDirection::Down | EDirection::Right) : (EDirection::Up | EDirection::Right);
        }
        else
        {
            return Vector.Y > 0 ? (EDirection::Down | EDirection::Left) : (EDirection::Up | EDirection::Left);
        }
    }
    else
    {
        return Vector.Y > 0 ? EDirection::Down : EDirection::Up;
    }
}

inline TVector2i ToVector2i(EDirection Direction)
{
    return ToVector2i(ToVector2(Direction));
}

template <typename T>
inline T DivCeil(T Value, T Divisor)
{
    static_assert(std::is_integral_v<T>);
    assert(Value >= 0 && Divisor > 0);
    return (Value + Divisor - 1) / Divisor;
}

inline s32 IncrementWrap(s32 Value, s32 Min, s32 Max)
{
    if (Value >= Max)
    {
        return Min;
    }
    return Value + 1;
}

inline s32 DecrementWrap(s32 Value, s32 Min, s32 Max)
{
    if (Value <= Min)
    {
        return Max;
    }
    return Value - 1;
}

inline s32 AddWrap(s32 Value, s32 AddAmt, s32 Min, s32 Max)
{
    Value = CLAMP(Value, Min, Max);
    const s32 Sum = Value + AddAmt;
    const s32 MinMaxDiff = (Max - Min) + 1;
    if (Sum > Max)
    {
        const s32 SumMaxDiff = Sum - (Max + 1); // adding 1 makes the rest simpler
        const s32 Remainder = SumMaxDiff % MinMaxDiff;
        return Min + Remainder;
    }
    else if (Sum < Min)
    {
        const s32 MinSumDiff = (Min - 1) - Sum;
        const s32 Remainder = MinSumDiff % MinMaxDiff;
        return Max - Remainder;
    }
    else
    {
        return Sum;
    }
}

inline bool IsPointInsideBox(TVector2 Point, TVector2 UpperLeft, TVector2 LowerRight)
{
    if (Point.X < UpperLeft.X || Point.X > LowerRight.X || Point.Y < UpperLeft.Y || Point.Y > LowerRight.Y)
    {
        return false;
    }
    return true;
}

inline float32 Respace(float32 X, TVector2 InRange, TVector2 OutRange)
{
    float32 NormVal;
    if (InRange.Y > InRange.X)
    {
        const float32 InRangeWidth = InRange.Y - InRange.X;
        NormVal = CLAMP((X - InRange.X) / InRangeWidth, 0.0f, 1.0f);
    } 
    else
    {
        const float32 InRangeWidth = InRange.X - InRange.Y;
        NormVal = CLAMP((X - InRange.Y) / InRangeWidth, 0.0f, 1.0f);
    }
    const float32 OutRangeSignedWidth = OutRange.Y - OutRange.X;
    return OutRange.X + OutRangeSignedWidth * NormVal;
}

inline TVector2 Lerp(TVector2 Begin, TVector2 End, float32 Alpha)
{
    return Begin + (End - Begin) * Alpha;
}

inline float32 Lerp(float32 Begin, float32 End, float32 Alpha)
{
    return Begin + (End - Begin) * Alpha;
}