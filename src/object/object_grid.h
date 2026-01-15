#pragma once

#include "../fundamental_defs.h"
#include "../math.h"
#include "../allocators/array.h"
#include "object.h"

using FForAllCellsVisitor = bool (*)(TVector2i CellXY, s32 CellIndex, void* UserData);

inline constexpr s32 OBJECT_GRID_CELL_SIDELENGTH = 1;
inline constexpr float32 INV_OBJECT_GRID_CELL_SIDELENGTH = 1.0f / float32(OBJECT_GRID_CELL_SIDELENGTH);

inline constexpr s32 OBJECT_GRID_SIDELENGTH_IN_CELLS = 256;
inline const TVector2i OBJECT_GRID_DIMENSIONS_IN_CELLS =
    TVector2i(OBJECT_GRID_SIDELENGTH_IN_CELLS, OBJECT_GRID_SIDELENGTH_IN_CELLS);
inline constexpr s32 OBJECT_GRID_CELL_COUNT =
    OBJECT_GRID_SIDELENGTH_IN_CELLS * OBJECT_GRID_SIDELENGTH_IN_CELLS;

inline constexpr float32 SHRINK_EXTENT_TO_BARELY_FIT_INTO_CELL = 0.1f;

class TObjectGrid
{
public:

    void Init();

    s32 WorldToGrid1d(TVector2 Position);

    TVector2i WorldToGrid2d(TVector2 Position);

    TVector2 Grid2dToWorld(TVector2i CellXY);

    TVector2 StickWorldPointToGrid(TVector2 Position);

    s32 Grid2dTo1d(TVector2i CellXY);

    TVector2i Grid1dTo2d(s32 Index);

    inline TVector2i ClampPointToGrid(TVector2i Point)
    {
        return Point.Clamp({0, 0}, OBJECT_GRID_DIMENSIONS_IN_CELLS - 1);
    }

    bool PlaceObjectIntoGrid(TObject* Object, bool bOverwriteGridBounds);

    void RemoveObjectFromGrid(TObject* Object);

    void ForAllCellsInBox(TVector2i CellBegin, TVector2i CellEnd, FForAllCellsVisitor Visitor, void* UserData = nullptr);

    void ForAllCellsInBounds(TObject* Object, FForAllCellsVisitor Visitor, void* UserData = nullptr);

    bool GetOccupiedGridCells(TObject* Object, TVector2i& OutBegin, TVector2i& OutEnd);

    void GetOccupiedGridCells(TVector2 Ul, TVector2 Lr, TVector2i& OutBegin, TVector2i& OutEnd);

    void GetCellsOnScreen(TVector2i& OutBegin, TVector2i& OutEnd);

    void NullifyObjectGridData(TObject* Object);
    

public:
// debug

    void DrawVisibleCells();

    void DrawOccupiedCells();

public:

    TArray<ECollisionChannel> CollisionChannelOccupancy;
    TArray<TDynamicArray<FObjectRef>> ObjectOccupancy;
    TVector2 UpperLeftOrigin;
};
