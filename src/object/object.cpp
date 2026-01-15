#include "object.h"
#include "../globals.h"
#include "../core.h"
#include "object_grid.h"

TObject *TObject::Get(FObjectRef Ref)
{
    assert(IsValid(Ref));
    return &Session.Objects[Ref.Index];
}

TObject *TObject::TryGet(FObjectRef& Ref, bool bAllowNullify)
{
    if (IsValid(Ref))
    {
        return &Session.Objects[Ref.Index];
    }
    else
    {
        // auto-nullify the ref
        if (bAllowNullify)
        {
            Ref = {};
        }
        return nullptr;
    }
}

TObject *TObject::TryGet(const FObjectRef &Ref)
{
    return IsValid(Ref) ? &Session.Objects[Ref.Index] : nullptr;
}

bool TObject::IsValid(FObjectRef Ref)
{
    return Session.Objects.TopIndex() >= Ref.Index
    && Session.Objects.IsItemInUse(Ref.Index)
    && Ref == Session.Objects[Ref.Index].Self;
}

namespace
{
    void InitCharacter(TObject* Obj)
    {
        Obj->Flags |= EObjectFlags::MoveFromInput;
        FGraphic& Graphic = Obj->Graphics.Push();
        SetAnimatedSprite(Obj, Graphic);
        Graphic.Extent = {0.5f, 0.5f};
        Graphic.bDoesCollide = true;
        Obj->CollisionChannels = ECollisionChannel::People;
    }

    void InitPlayer(TObject* Obj)
    {
        InitCharacter(Obj);
        Obj->Character.HP = 100;
    }

    void SetNpcStateCdf(TObject* Obj, TArray<FNpcStateCdfEntry>* StateCdf, TVector2 Origin, TVector2 Extent)
    {
        assert(IsPointInsideBox(Obj->GetPosition(), Origin - Extent, Origin + Extent));
        Obj->Npc.StateProbabilityCdf = StateCdf;
        Obj->Npc.MoveOrigin = Origin;
        Obj->Npc.MoveExtent = Extent;
        RandomDrawNpcState(Obj);
        InitMoveDirectionCdf(Obj);
    }

    void InitNPC(TObject* Obj)
    {
        InitCharacter(Obj);
        switch (Obj->SubType.NpcType)
        {
        // case ENpcType::Butterfly: 
        //     {
        //         Obj->BaseDepth = -1;
        //     }
        //     break;
        case ENpcType::SomeGuy:
            SetNpcStateCdf(Obj, &NpcStateCdf::SomeGuy, {}, Session.LevelExtent);
            Obj->Movement.Speed = FObjectMovement::PLAYER_BASE_SPEED * 0.5f;
            Obj->Character.HP = 5;
            Obj->Flags |= EObjectFlags::IsEnemy;
            break;
        }
    }
}

void TObject::Init()
{
    switch (Self.Type)
    {
    case EObjectType::None:
        LOG_ERROR("tried to instantiate object of type None");
        assert(false);
        break;
    case EObjectType::Player:
        InitPlayer(this);
        break;
    case EObjectType::Camera:
        break;
    case EObjectType::Environment:
        break;
    case EObjectType::NPC:
        InitNPC(this);
        break;
    case EObjectType::OneOffEffect:
        break;
    case EObjectType::Teleporter:
        break;
    }
    Orientation = EDirection::Down;
}

void TObject::Release()
{
    if (!HasFlag(EObjectFlags::ReleaseWithoutDeath))
    {
        Die();
    }

    switch (Self.Type)
    {
    case EObjectType::None:
        LOG_ERROR("tried to release object of type None");
        assert(false);
        break;
    case EObjectType::Player:
        break;
    case EObjectType::Camera:
        break;
    case EObjectType::Environment:
        break;
    case EObjectType::NPC:
        break;
    case EObjectType::OneOffEffect:
        break;
    case EObjectType::Teleporter:
        break;
    }

    DetachFromParent();
    DetachChildren();

    Session.ObjectGrid->RemoveObjectFromGrid(this);
    Attachments.Free();
    Graphics.Free();
}

TVector2 TObject::GetPosition() const
{
    TVector2 Pos = Position;
    if (Self.Type == EObjectType::Camera)
    {
        // so camera pos isn't upper left of each cell, but the center
        // places the subject at the center of the view
        Pos += TVector2(0.5, 0.5); 
    }
    if (TObject* Parent = TryGetParent())
    {
        Pos += Parent->Position;
    }
    return Pos;
}

void TObject::AttachToParent(FObjectRef Ref, TVector2 Offset)
{
    DetachFromParent();
    TObject* NewAttachParent = TObject::Get(Ref);
    NewAttachParent->Attachments.Push(Self);
    AttachParent = Ref;
    Position = Offset;
    Flags |= EObjectFlags::MoveAsAttachment;
}

void TObject::DetachFromParent()
{
    if (IsValid(AttachParent))
    {
        TObject* CurAttachParent = TObject::Get(AttachParent);
        CurAttachParent->Attachments.RemoveUnordered(Self);
        Position = CurAttachParent->Position;
    }
    Movement = {}; // movement is inherited when attached
    AttachParent = {};
    Flags &= ~EObjectFlags::MoveAsAttachment;
}

void TObject::DetachChildren()
{
    // iterating backwards because children remove themselves from this array during DetachFromParent()
    for (int i = Attachments.Count() - 1; i >= 0; i--)
    {
        TObject* Child = TObject::TryGet(Attachments[i]);
        if (Child != nullptr)
        {
            Child->DetachFromParent();
        }
    }
    Attachments.Free();
}

TObject *TObject::TryGetParent()
{
    TObject* Parent = nullptr;
    if (HasFlag(EObjectFlags::MoveAsAttachment))
    {
        Parent = TObject::TryGet(AttachParent, false);
        if (Parent == nullptr)
        {
            DetachFromParent();
        }
    }
    return Parent;
}

TObject *TObject::TryGetParent() const
{
    TObject* Parent = nullptr;
    if (HasFlag(EObjectFlags::MoveAsAttachment))
    {
        Parent = TObject::TryGet(AttachParent);
    }
    return Parent;
}

void TObject::Die()
{
}

void TObject::SetPosition(TVector2 NewPosition)
{
    Session.ObjectGrid->RemoveObjectFromGrid(this);
    Position = NewPosition;
    Session.ObjectGrid->PlaceObjectIntoGrid(this, true);
}

namespace
{
    inline void GetCornersImpl(const FGraphic* Graphic, TVector2& OutUl, TVector2& OutLr, TVector2 Offset={}, float32 ExtentScale=1, EViewSpace OutSpace=EViewSpace::World)
    {
        OutUl = ConvertSpace(Graphic->Offset + Offset, Graphic->Space, OutSpace);
        OutLr = ConvertSpace(Graphic->Offset + Offset + Graphic->Extent * 2 * ExtentScale, Graphic->Space, OutSpace);
    }
}

void TObject::GetCorners(TVector2 &OutUpperLeft, TVector2 &OutLowerRight, EViewSpace OutSpace, float32 ExtentScale) const
{
    if (PrimaryGraphic() != nullptr)
    {
        GetCornersImpl(PrimaryGraphic(), OutUpperLeft, OutLowerRight, GetPosition(), ExtentScale, OutSpace);
    }
}

