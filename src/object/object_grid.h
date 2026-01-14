#pragma once

#include "../fundamental_defs.h"
#include "../math.h"
#include "../allocators/array.h"
#include "object.h"

inline constexpr s32 OBJECT_GRID_SIDELENGTH_IN_CELLS = 256;
inline const TVector2i OBJECT_GRID_DIMENSIONS_IN_CELLS = TVector2i(OBJECT_GRID_SIDELENGTH_IN_CELLS, OBJECT_GRID_SIDELENGTH_IN_CELLS);
inline const s32 OBJECT_GRID_CELL_COUNT = OBJECT_GRID_SIDELENGTH_IN_CELLS * OBJECT_GRID_SIDELENGTH_IN_CELLS;

inline const float32 SHRINK_EXTENT_TO_BARELY_FIT_INTO_CELL = 0.001f;

class ObjectGrid
{
public:

    void Init();

    s32 WorldToGrid1d(TVector2 Position);

    s32 WorldToGrid2d(TVector2 Position);

    s32 Grid2dToWorld(TVector2 Position);

    // inline TVector2i ClampPoint(TVector2i Point) { return CLAMP(Point, TVector2i(), OBJECT_GRID_DIMENSIONS_IN_CELLS-1); }

public:

    TArray<ECollisionChannel> CollisionChannelOccupancy;
    TArray<TDynamicArray<FObjectRef>> ObjectOccupancy;
    TVector2 UpperLeftOrigin;
};