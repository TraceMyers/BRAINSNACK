#pragma once

#include "session.h"
#include "graphics/renderer.h"
#include "fundamental_defs.h"
#include "allocators/allocator.h"
#include "allocators/cstd_allocator.h"
#include "allocators/super_allocator.h"
#include "allocators/bump_allocator.h"
#include "allocators/no_allocator.h"
#include "allocators/pool.h"
#include "object/object.h"
#include "object/movement.h"
#include "input.h"

// ---------------------------------------------------------------------------------------------------------------------

extern TSession Session;
extern TRenderer Renderer;
extern TInput Input;
extern TObjectMovement ObjectMovement;
extern FObjectRef DebugRef;

// ---------------------------------------------------------------------------------------------------------------------

inline void* RuntimeAlloc(u32 Size)
{
    return Session.RuntimeAllocator.Alloc(Size);
}

inline void* TempAlloc(u32 Size)
{
    return Session.FrameAllocator.Alloc(Size);
}

// ---------------------------------------------------------------------------------------------------------------------
