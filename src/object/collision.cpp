#include "collision.h"
#include "object_grid.h"

void Collision::ProcessPlayerCollisions(float DeltaTime)
{
    TObject* Player = TObject::Get(Session.PlayerID);

    if (!Player->HasFlag(EObjectFlags::PlacedInGrid))
    {
        return;
    }
    if (Player->Character.TakeHitCooldown > 0)
    {
        Player->Character.TakeHitCooldown -= DeltaTime;
        return;
    }

    const TVector2i SearchUL = Player->GridUpperLeft - TVector2i(3, 3);
    const TVector2i SearchLR = Player->GridLowerRight + TVector2i(3, 3);

    TDynamicArray<FObjectRef> Neighbors;
    Neighbors.TempInit(256);
    Session.ObjectGrid->ForAllCellsInBox(SearchUL, SearchLR, CellVisitor_CollectObjectsInCell, &Neighbors);

    TVector2 PlayerUL, PlayerLR;
    TVector2 NeighborUL, NeighborLR;
    Player->GetCorners(PlayerUL, PlayerLR, EViewSpace::World, 0.25f);

    for (s32 i = 0; i < Neighbors.Count(); i++)
    {
        FObjectRef NeighRef = Neighbors[i];
        TObject* Neighbor = TObject::TryGet(NeighRef);
        if (Neighbor == nullptr)
        {
            continue;
        }
        if (!Neighbor->HasFlag(EObjectFlags::IsEnemy))
        {
            continue;
        }
        Neighbor->GetCorners(NeighborUL, NeighborLR, EViewSpace::World, 0.25f);
        if (DoBoxesOverlap(PlayerUL, PlayerLR, NeighborUL, NeighborLR))
        {
            constexpr float TEST_PUSH_AMT = 9.0f;
            const float32 PushOnPlayer = Neighbor->Character.Mass * TEST_PUSH_AMT;
            const float32 PushOnNeighbor = Player->Character.Mass * TEST_PUSH_AMT;
            const TVector2 PushNorm = (Neighbor->GetPosition() - Player->GetPosition()).Normal();
            Player->Character.PhysicsVelocity += PushNorm * (PushOnPlayer * -1.0f);
            Neighbor->Character.PhysicsVelocity += PushNorm * PushOnNeighbor;
            Player->Character.TakeHitCooldown = 0.8f;
            break;
        }
    }
}

bool Collision::CellVisitor_CollectObjectsInCell(TVector2i CellXY, s32 CellIndex, void *UserData)
{
    TDynamicArray<FObjectRef>& Neighbors = *(TDynamicArray<FObjectRef>*)UserData;
    TDynamicArray<FObjectRef>& CellOccupancy = Session.ObjectGrid->ObjectOccupancy[CellIndex];
    for (s32 i = 0; i < CellOccupancy.Count(); i++)
    {
        Neighbors.PushUnique(CellOccupancy[i]);
    }
    return true;
}
