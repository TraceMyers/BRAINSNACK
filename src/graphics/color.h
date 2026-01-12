#pragma once

#include "../fundamental_defs.h"

struct Color4b
{
    u8 r = 0;
    u8 g = 0;
    u8 b = 0;
    u8 a = 255;
};

struct Color4f
{
    static Color4f White() { return {1,1,1,1}; }
    static Color4f Black() { return {0,0,0,1}; }
    static Color4f Blue() { return {0,0,1,1}; }
    static Color4f LightGrey() { return {0.9f, 0.9f, 0.9f, 1.0f}; }
    static Color4f DarkGrey() { return {0.1f, 0.1f, 0.1f, 1.0f}; }
    inline bool operator == (const Color4f& Other)
    {
        return memcmp(this, &Other, sizeof(Color4f)) == 0;
    }
    inline bool operator != (const Color4f& Other)
    {
        return memcmp(this, &Other, sizeof(Color4f)) != 0;
    }

    float32 r = 0;
    float32 g = 0;
    float32 b = 0;
    float32 a = 1;
};

inline Color4f ToColor4f(const Color4b Color)
{
    constexpr float32 ONE_OVER_255 = 1.0f / 255.0f;
    return {
        (float32)Color.r * ONE_OVER_255, 
        (float32)Color.g * ONE_OVER_255,
        (float32)Color.b * ONE_OVER_255,
        (float32)Color.a * ONE_OVER_255
    };
}

// probably not the best way to brighten a color but it will do
inline Color4f Brighten(const Color4f& Color, float Amount)
{
    return {
        MIN(Color.r + Amount, 1.0f),
        MIN(Color.g + Amount, 1.0f),
        MIN(Color.b + Amount, 1.0f),
        Color.a
    };
}

inline Color4b ToColor4b(const Color4f& Color)
{
    return {
        (u8)(Color.r * 255.0f), 
        (u8)(Color.g * 255.0f),
        (u8)(Color.b * 255.0f),
        (u8)(Color.a * 255.0f)
    };   
}