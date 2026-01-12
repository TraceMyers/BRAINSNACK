#pragma once

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>
#include <type_traits>

#include "fundamental_defs.h"
#include "time.h"
#include "math.h"
#include "object/object.h"
#include "object/camera.h"
#include "object/ai.h"
#include "graphics/color.h"
#include "graphics/graphic.h"
#include "allocators/array.h"
#include "allocators/allocator.h"
#include "allocators/bump_allocator.h"
#include "allocators/cstd_allocator.h"
#include "allocators/super_allocator.h"
#include "allocators/no_allocator.h"
#include "allocators/bit_array.h"
#include "allocators/pool.h"
#include "session.h"
#include "input.h"
#include "graphics/renderer.h"
#include "string.h"
#include "print.h"
#include "modifier.h"
#include "generated.h"

template<typename T>
inline bool IsPow2(T Value)
{
    return Value > 0 && ((Value & (Value - 1)) == 0);
}

template<typename T>
inline bool IsMultipleOfPow2(T Value, T PowerOf2)
{
    assert(IsPow2(PowerOf2));
    const T BitsBelow = (PowerOf2 - 1);
    return Value != 0 && (Value & BitsBelow) == 0;
}

template<typename T>
inline T CeilToMultipleOfPow2(T Value, T PowerOf2)
{
    if (IsMultipleOfPow2(Value, PowerOf2))
    {
        return Value;
    }
    else
    { 
        const T BitsBelow = (PowerOf2 - 1);
        return Value + (PowerOf2 - (Value & BitsBelow));
    }
}

// ceil to the next highest power of 2. if already a power of 2, no change.
inline u32 CeilToPow2(u32 Value)
{
    Value--;
    Value |= Value >> 1;
    Value |= Value >> 2;
    Value |= Value >> 4;
    Value |= Value >> 8;
    Value |= Value >> 16;
    return Value + 1;
}

// if already a power of 2, move up to the next. otherwise, just ceil to the next highest power of 2
inline u32 CeilToNextPow2(u32 Value)
{
    if (IsPow2(Value))
    {
        return Value << 1;
    }
    else
    {
        return CeilToPow2(Value);
    }
}

// for use with the macro below to make it easy to defer a simple member fn call
template<typename T>
class TDeferCall
{
public:

    TDeferCall(T* InObject, void (T::*InCallback)())
    {
        assert(InObject != nullptr);
        Object = InObject;
        Callback = InCallback;
    }

    ~TDeferCall() {
        (*Object.*Callback)();
    }

public:

    T* Object;
    void (T::*Callback)();
};

// if a class Thing has a member function DoStuff(void), type DEFER_CALL(Thing, DoStuff) to defer calling Thing->DoStuff until the end of the scope
#define DEFER_CALL(Object, RunLater) \
    using BookendType ## Object = decltype(Object); \
    TDeferCall<BookendType ## Object> __ScopeBookend_ ## Object (&Object, &BookendType ## Object::RunLater);

