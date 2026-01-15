#include "object_grid.h"
#include "../globals.h"
#include "camera.h"

namespace
{
    struct FCellVisitData
    {
        TObject* Object = nullptr;
        bool bFailure = false;
    };

    struct FCellVisitContext
    {
        TObjectGrid* Grid = nullptr;
        FCellVisitData* Visit = nullptr;
    };

    // static inline bool IsCellAvailableFor(const TObjectGrid* Grid, const TObject* Obj, s32 CellIndex)
    // {
    //     if (Obj->Graphics.Count() == 0)
    //     {
    //         return true;
    //     }
    //     const ECollisionChannel Collision = Obj->CollisionChannels;
    //     return (Collision & Grid->CollisionChannelOccupancy[CellIndex]) == ECollisionChannel::None;
    // }

    // static bool ConfirmAvailabilityVisitorCtx(TVector2i /*CellXY*/, s32 CellIndex, void* UserData)
    // {
    //     FCellVisitContext* Ctx = (FCellVisitContext*)UserData;
    //     if (!IsCellAvailableFor(Ctx->Grid, Ctx->Visit->Object, CellIndex))
    //     {
    //         Ctx->Visit->bFailure = true;
    //         return false;
    //     }
    //     return true;
    // }

    static bool AddToCellVisitorCtx(TVector2i /*CellXY*/, s32 CellIndex, void* UserData)
    {
        FCellVisitContext* Ctx = (FCellVisitContext*)UserData;
        TObject* Obj = Ctx->Visit->Object;

        // assert(IsCellAvailableFor(Ctx->Grid, Obj, CellIndex));

        if (Obj->Graphics.Count() != 0)
        {
            Ctx->Grid->CollisionChannelOccupancy[CellIndex] |= Obj->CollisionChannels;
            TDynamicArray<FObjectRef>& Occupancy = Ctx->Grid->ObjectOccupancy[CellIndex];
            assert(!Occupancy.Contains(Obj->Self));
            if (Occupancy.Count() == 0)
            {
                Occupancy.SetMax(16);
            }
            Occupancy.Push(Obj->Self);
        }

        return true;
    }

    static bool RemoveFromCellsVisitorCtx(TVector2i /*CellXY*/, s32 CellIndex, void* UserData)
    {
        FCellVisitContext* Ctx = (FCellVisitContext*)UserData;
        TObject* Obj = Ctx->Visit->Object;

        if (Obj->Graphics.Count() != 0)
        {
            Ctx->Grid->CollisionChannelOccupancy[CellIndex] &= ~Obj->CollisionChannels;
            TDynamicArray<FObjectRef>& Occupancy = Ctx->Grid->ObjectOccupancy[CellIndex];
            const bool bRemoved = Occupancy.RemoveUnordered(Obj->Self);
            assert(bRemoved);
            if (Occupancy.Count() == 0)
            {
                Occupancy.Free();
            }
        }

        return true;
    }

    static void DrawCellQuadImpl(FCellVisitContext* Ctx, TVector2i CellXY, s32 CellIndex, const Color4f& Color=Color4f::White())
    {
        TVector2 UpperLeft = Ctx->Grid->Grid2dToWorld(CellXY);
        TVector2 LowerRight = Ctx->Grid->Grid2dToWorld(CellXY + TVector2i(1,1));
        TVector2 UpperLeftInWindow = WorldToWindowPoint(UpperLeft + TVector2(0.1f));
        TVector2 LowerRightInWindow = WorldToWindowPoint(LowerRight - TVector2(0.1f));
        const FQuad Quad = {UpperLeftInWindow, LowerRightInWindow};
        Renderer.DrawQuad(Quad, {}, {}, Color);
    }

    static bool DrawCellQuad(TVector2i CellXY, s32 CellIndex, void* UserData)
    {
        constexpr float OFFSET = 0.8f;
        FCellVisitContext* Ctx = (FCellVisitContext*)UserData;
        DrawCellQuadImpl(Ctx, CellXY, CellIndex, {0, 1, 0, 1});
        return true;
    }

    static bool DrawCellQuadIfOccupied(TVector2i CellXY, s32 CellIndex, void* UserData)
    {
        constexpr float OFFSET = 0.8f;
        FCellVisitContext* Ctx = (FCellVisitContext*)UserData;
        TDynamicArray<FObjectRef>& Occupancy = Ctx->Grid->ObjectOccupancy[CellIndex];
        const float32 GreenAmt = CLAMP((float32)Occupancy.Count() / 16.0f, 0.0f, 1.0f);
        const float32 BlueAmt = 1.0f - GreenAmt;
        if (GreenAmt > 0)
        {
            DrawCellQuadImpl(Ctx, CellXY, CellIndex, {0, GreenAmt, BlueAmt, 1});
        }
        return true;
    }
}

void TObjectGrid::Init()
{
    CollisionChannelOccupancy.Init(OBJECT_GRID_CELL_COUNT, &Session.RuntimeAllocator);
    ObjectOccupancy.Init(OBJECT_GRID_CELL_COUNT, &Session.RuntimeAllocator);
    const float32 SideLength = float32(OBJECT_GRID_CELL_SIDELENGTH * OBJECT_GRID_SIDELENGTH_IN_CELLS);
    UpperLeftOrigin = TVector2(-SideLength, -SideLength) * 0.5f;
}

s32 TObjectGrid::WorldToGrid1d(TVector2 Position)
{
    const TVector2i XY = WorldToGrid2d(Position);
    return Grid2dTo1d(XY);
}

TVector2i TObjectGrid::WorldToGrid2d(TVector2 Position)
{
    const TVector2 RelativePos = Position - UpperLeftOrigin;
    const TVector2 Scaled = RelativePos * INV_OBJECT_GRID_CELL_SIDELENGTH;
    return ClampPointToGrid(ToVector2i(Scaled));
}

TVector2 TObjectGrid::Grid2dToWorld(TVector2i CellXY)
{
    const TVector2 RelativePos = ToVector2(CellXY * OBJECT_GRID_CELL_SIDELENGTH);
    return RelativePos + UpperLeftOrigin;
}

TVector2 TObjectGrid::StickWorldPointToGrid(TVector2 Position)
{
    const TVector2i GridPos = WorldToGrid2d(Position);
    return Grid2dToWorld(GridPos);
}

s32 TObjectGrid::Grid2dTo1d(TVector2i CellXY)
{
    assert(CellXY.X >= 0 && CellXY.X < OBJECT_GRID_SIDELENGTH_IN_CELLS);
    assert(CellXY.Y >= 0 && CellXY.Y < OBJECT_GRID_SIDELENGTH_IN_CELLS);
    return CellXY.Y * OBJECT_GRID_SIDELENGTH_IN_CELLS + CellXY.X;
}

TVector2i TObjectGrid::Grid1dTo2d(s32 Index)
{
    TVector2i Cell;
    Cell.Y = Index / OBJECT_GRID_SIDELENGTH_IN_CELLS;
    Cell.X = Index - Cell.Y * OBJECT_GRID_SIDELENGTH_IN_CELLS;
    assert(Cell.X >= 0 && Cell.Y >= 0 && Cell.Y < OBJECT_GRID_SIDELENGTH_IN_CELLS);
    return Cell;
}

void TObjectGrid::ForAllCellsInBox(TVector2i CellBegin, TVector2i CellEnd, FForAllCellsVisitor Visitor, void* UserData)
{
    for (s32 Y = CellBegin.Y; Y <= CellEnd.Y; Y++)
    {
        for (s32 X = CellBegin.X; X <= CellEnd.X; X++)
        {
            const s32 Index = Grid2dTo1d(TVector2i(X, Y));
            if (!Visitor(TVector2i(X, Y), Index, UserData))
            {
                return;
            }
        }
    }
}

void TObjectGrid::ForAllCellsInBounds(TObject* Object, FForAllCellsVisitor Visitor, void* UserData)
{
    TVector2i Begin, End;
    if (GetOccupiedGridCells(Object, Begin, End))
    {
        ForAllCellsInBox(Begin, End, Visitor, UserData);
    }
}

bool TObjectGrid::GetOccupiedGridCells(TObject* Object, TVector2i& OutBegin, TVector2i& OutEnd)
{
    FGraphic* Graphic = Object->PrimaryGraphic();
    const bool bCanCollide = Graphic != nullptr && Graphic->bDoesCollide;

    const TVector2 Extent = Graphic != nullptr ? Graphic->Extent : TVector2(0.0f, 0.0f);

    if (!bCanCollide || Extent.X <= (SHRINK_EXTENT_TO_BARELY_FIT_INTO_CELL * 2.0f) || Extent.Y <= (SHRINK_EXTENT_TO_BARELY_FIT_INTO_CELL * 2.0f))
    {
        OutBegin = {};
        OutEnd = {};
        return false;
    }

    TVector2 UpperLeft;
    TVector2 LowerRight;
    Object->GetCorners(UpperLeft, LowerRight);
    GetOccupiedGridCells(UpperLeft, LowerRight, OutBegin, OutEnd);
    return true;
}

void TObjectGrid::GetOccupiedGridCells(TVector2 Ul, TVector2 Lr, TVector2i& OutBegin, TVector2i& OutEnd)
{
    const TVector2i CellUl = WorldToGrid2d(Ul + SHRINK_EXTENT_TO_BARELY_FIT_INTO_CELL);
    const TVector2i CellLr = WorldToGrid2d(Lr - SHRINK_EXTENT_TO_BARELY_FIT_INTO_CELL);
    assert(CellUl.X <= CellLr.X);
    assert(CellUl.Y <= CellLr.Y);
    OutBegin = CellUl;
    OutEnd = CellLr;
}

void TObjectGrid::GetCellsOnScreen(TVector2i &OutBegin, TVector2i &OutEnd)
{
    const TVector2 WindowUl_World = WindowToWorldPoint({0,0});
    const TVector2 WindowLr_World = WindowToWorldPoint(Renderer.WindowExtent() * 2);
    OutBegin = WorldToGrid2d(WindowUl_World);
    OutEnd   = WorldToGrid2d(WindowLr_World);
}

void TObjectGrid::NullifyObjectGridData(TObject* Object)
{
    Object->GridUpperLeft = {};
    Object->GridLowerRight = {};
    Object->Flags &= ~EObjectFlags::PlacedInGrid;
}

void TObjectGrid::DrawVisibleCells()
{
    FCellVisitData Visit;
    FCellVisitContext Ctx = {this, &Visit};
    TVector2i BeginCells, EndCells;
    GetCellsOnScreen(BeginCells, EndCells);
    // uncomment if you want to see the cells on the border not be drawn
    // BeginCells += TVector2i(1,1);
    // EndCells -= TVector2i(1, 1);
    ForAllCellsInBox(BeginCells, EndCells, DrawCellQuad, &Ctx);
}

void TObjectGrid::DrawOccupiedCells()
{
    FCellVisitData Visit;
    FCellVisitContext Ctx = {this, &Visit};
    TVector2i BeginCells, EndCells;
    GetCellsOnScreen(BeginCells, EndCells);
    ForAllCellsInBox(BeginCells, EndCells, DrawCellQuadIfOccupied, &Ctx);
}

bool TObjectGrid::PlaceObjectIntoGrid(TObject* Object, bool bOverwriteGridBounds)
{
    // if object takes up no grid space, placement is considered successful.
    FCellVisitData Visit;
    Visit.Object = Object;
    Visit.bFailure = false;

    FCellVisitContext Ctx;
    Ctx.Grid = this;
    Ctx.Visit = &Visit;

    TVector2i CellBegin, CellEnd;
    const bool bOccupiesAnyCells = GetOccupiedGridCells(Object, CellBegin, CellEnd);

    if (bOccupiesAnyCells)
    {
        // ForAllCellsInBox(CellBegin, CellEnd, ConfirmAvailabilityVisitorCtx, &Ctx);

        // if (!Visit.bFailure)
        {
            ForAllCellsInBox(CellBegin, CellEnd, AddToCellVisitorCtx, &Ctx);

            if (!Visit.bFailure)
            {
                if (bOverwriteGridBounds)
                {
                    Object->GridUpperLeft = CellBegin;
                    Object->GridLowerRight = CellEnd;
                }
                Object->Flags |= EObjectFlags::PlacedInGrid;
            }
        }
    }

    return !Visit.bFailure;
}

void TObjectGrid::RemoveObjectFromGrid(TObject* Object)
{
    FCellVisitData Visit;
    Visit.Object = Object;
    Visit.bFailure = false;

    FCellVisitContext Ctx;
    Ctx.Grid = this;
    Ctx.Visit = &Visit;

    if ((Object->Flags & EObjectFlags::PlacedInGrid) != EObjectFlags::None)
    {
        const TVector2i Diff = Object->GridLowerRight - Object->GridUpperLeft;
        assert(Diff.X >= 0 && Diff.Y >= 0);
        ForAllCellsInBox(Object->GridUpperLeft, Object->GridLowerRight, RemoveFromCellsVisitorCtx, &Ctx);
    }

    NullifyObjectGridData(Object);
}
