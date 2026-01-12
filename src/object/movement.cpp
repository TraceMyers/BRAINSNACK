#include "movement.h"
#include "../globals.h"

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
        MoveDueToInput(Obj, DeltaTime);
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

void TObjectMovement::MoveDueToInput(TObject *Object, float DeltaTime)
{
    Object->Flags &= ~TRACKING_MOVEMENT_FLAGS;

    // todo: grid stuff
    const bool bTrackedInGrid = Object->HasFlag(EObjectFlags::PlacedInGrid);

    if (!bTrackedInGrid)
    {

    }
    if (Object->HasFlags(EObjectFlags::MoveAsAttachment | EObjectFlags::SkipMoveUpdate | EObjectFlags::IsEditorGoober))
    {
        return;
    }
    if (!Object->HasFlag(EObjectFlags::MoveFromInput))
    {
        return;
    }
    // todo: interaction

    if (Object->Movement.bMoving)
    {
        Object->Orientation = Object->Movement.Direction;
        const TVector2 NewPosition = Object->Position + ToVector2(Object->Movement.Direction) * Object->Movement.Speed * DeltaTime;
        // todo: grid stuff
        if (Object->Self.Type == EObjectType::NPC)
        {
            const TVector2 HardTetherUL = Object->Npc.MoveOrigin - Object->Npc.MoveExtent;
            const TVector2 HardTetherLR = Object->Npc.MoveOrigin + Object->Npc.MoveExtent;
            if (IsPointInsideBox(NewPosition, HardTetherUL, HardTetherLR))
            {
                Object->Position = NewPosition;
                Object->Flags |= EObjectFlags::AnimateMovement;
            }
        }
        else
        {
            Object->Position = NewPosition;
            Object->Flags |= EObjectFlags::AnimateMovement;
        }
    }
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
