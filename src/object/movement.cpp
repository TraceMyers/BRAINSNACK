#include "movement.h"
#include "../globals.h"
#include "object_grid.h"

namespace
{
    inline static constexpr EObjectFlags TRACKING_MOVEMENT_FLAGS = EObjectFlags::JustFinishedMove | EObjectFlags::AnimateMovement | EObjectFlags::JustStartedMove;
}

void TObjectMovement::FrameUpdate(float DeltaTime)
{
    if (Session.Mode != ESessionMode::PlayGame)
    {
        return;
    }
    if (bSkipUpdate)
    {
        return;
    }

    TVector2 InputMoveDelta;
    TVector2 PhysicsMoveDelta;

    for (int i = 0; i < Session.Objects.TopIndex()+1; i++)
    {
        if (!Session.Objects.IsItemInUse(i))
        {
            continue;
        }
        TObject* Obj = &Session.Objects[i];
        if (!Obj->CanEverMove())
        {
            continue;
        }

        bool bDoMove = false;
        bDoMove |= MoveDueToInput(Obj, InputMoveDelta, DeltaTime);
        bDoMove |= MoveDueToPhysics(Obj, PhysicsMoveDelta, DeltaTime);

        if (bDoMove)
        {
            const TVector2 NewPosition = Obj->GetPosition() + InputMoveDelta + PhysicsMoveDelta;
            TVector2 Clamped = NewPosition;
            if (Obj->Self.Type == EObjectType::NPC)
            {
                const TVector2 HardTetherUL = Obj->Npc.MoveOrigin - Obj->Npc.MoveExtent;
                const TVector2 HardTetherLR = Obj->Npc.MoveOrigin + Obj->Npc.MoveExtent;
                Clamped = NewPosition.Clamp(HardTetherUL, HardTetherLR);
            }
            Obj->SetPosition(Clamped);
            Obj->Flags |= EObjectFlags::AnimateMovement;
        }
    }
    for (int i = 0; i < Session.Objects.TopIndex()+1; i++)
    {
        if (!Session.Objects.IsItemInUse(i))
        {
            continue;
        }
        TObject* Obj = &Session.Objects[i];
        if (!Obj->CanEverMove())
        {
            continue;
        }
        MoveDueToAttachment(Obj, DeltaTime);
    }
}

bool TObjectMovement::MoveDueToInput(TObject *Object, TVector2& OutMoveDelta, float DeltaTime)
{
    Object->Flags &= ~TRACKING_MOVEMENT_FLAGS;
    bool bTrackedInGrid = Object->HasFlag(EObjectFlags::PlacedInGrid);

    if (Object->HasFlag(EObjectFlags::MoveAsAttachment))
    {
        if (bTrackedInGrid)
        {
            Session.ObjectGrid->RemoveObjectFromGrid(Object);
        }
        return false;
    }
    if (!bTrackedInGrid)
    {
        bTrackedInGrid = Session.ObjectGrid->PlaceObjectIntoGrid(Object, true);
    }
    if (Object->HasFlags(EObjectFlags::MoveAsAttachment | EObjectFlags::SkipMoveUpdate | EObjectFlags::IsEditorGoober))
    {
        return false;
    }
    if (!Object->HasFlag(EObjectFlags::MoveFromInput))
    {
        return false;
    }

    if (Object->Movement.bMoving)
    {
        Object->Orientation = Object->Movement.Direction;
        OutMoveDelta = ToVector2(Object->Movement.Direction) * Object->Movement.Speed * DeltaTime;
        return true;
    }
    else
    {
        return false;
    }
}

bool TObjectMovement::MoveDueToPhysics(TObject *Object, TVector2& OutMoveDelta, float DeltaTime)
{
    bool bMovement = false;
    const float32 SpeedSq = Object->Character.PhysicsVelocity.MagnitudeSq();
    if (SpeedSq > 1e-6f)
    {
        bMovement = true;
        OutMoveDelta += Object->Character.PhysicsVelocity * DeltaTime;
        const float32 Speed = sqrtf(SpeedSq);
        const float32 PushDecay = Object->Character.Mass * 40.0f * DeltaTime;
        if (Speed < PushDecay)
        {
            Object->Character.PhysicsVelocity = {};
        }
        else
        {
            const TVector2 PhysVelocityNorm = Object->Character.PhysicsVelocity / Speed;
            LOG("push decay %", PushDecay);
            Object->Character.PhysicsVelocity -= PhysVelocityNorm * PushDecay;
        }
    }
    else
    {
        Object->Character.PhysicsVelocity = {};
    }
    return bMovement;
}

void TObjectMovement::MoveDueToAttachment(TObject *Object, float DeltaTime)
{
    if (!Object->HasFlag(EObjectFlags::MoveAsAttachment))
    {
        return;
    }

    TObject* AttachParent = Object->TryGetParent();
    if (AttachParent == nullptr)
    {
        return;
    }

    Object->Flags |= AttachParent->Flags & TRACKING_MOVEMENT_FLAGS; 
    Object->Movement = AttachParent->Movement;
}
