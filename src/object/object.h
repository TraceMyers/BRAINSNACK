#pragma once

#include "../fundamental_defs.h"
#include "../math.h"
#include "../allocators/array.h"
#include "../graphics/graphic.h"

enum class EObjectFlags : u32
{
    None                    = 0,
    PlacedInGrid            = 1 << 0,
    AnimateMovement         = 1 << 1,
    JustStartedMove         = 1 << 2,
    JustFinishedMove        = 1 << 3,
    MoveFromInput           = 1 << 4,
    MoveAsAttachment        = 1 << 5,
    SkipMoveUpdate          = 1 << 6,
    SkipAIUpdate            = 1 << 7,
    IsEditorGoober          = 1 << 8
};

// enable bitwise operators
template<>
inline constexpr bool bEnableEnumFlags<EObjectFlags> = true;

struct FObjectAttachment
{
    FObjectRef ObjectRef;
    TVector2 Offset;
};

enum class EOverlapChannel : u8
{
    None                = 0,
    Enemies             = 1 << 0
};

// enable bitwise operators
template<>
inline constexpr bool bEnableEnumFlags<EOverlapChannel> = true;

enum class ECollisionChannel : u8
{
    None                = 0,
    People              = 1 << 0,
    Enemies             = 1 << 1,
    Ephemera            = 1 << 2
};

// enable bitwise operators
template<>
inline constexpr bool bEnableEnumFlags<ECollisionChannel> = true;

enum class ENpcType : u8
{
    SomeGuy
};

enum class EOneOffEffectType : u8
{
    Heart,
    Snooze,
    Turkey,
    NetSwing
};

enum class EEnvironmentType : u8
{
    LongHouse,
    TreeA,
    TreeB,
    TreeC,
    TreeD
};

struct FObjectMovement {
    static constexpr float32 PLAYER_BASE_SPEED = 4.2f;
    float32 Speed = PLAYER_BASE_SPEED;
    EDirection Direction;
    bool bMoving;
};

enum class ENpcState : u8
{
    None,
    Pause,
    RandomWalk,
    MoveTowardPlayer,
    KeepDistanceFromPlayer
};

struct FNpcStateCdfEntry
{
    ENpcState State;
    float32 Weight;
    float32 StateTimeMin;
    float32 StateTimeMax;
};

struct FNpcRandomWalk
{
    float32 MoveDirectionCdf[4];
    float32 MoveDirectionCdfReinitCooldown;
};

struct FCharacter
{
    float32 HP;
};

struct FNpc
{
    ENpcState State;
    FNpcRandomWalk RandomWalk;
    TArray<FNpcStateCdfEntry>* StateProbabilityCdf;
    float32 StateTimer;
    float32 KeepDistanceFromPlayer = 5.0f;
    TVector2 MoveOrigin;
    TVector2 MoveExtent;
};

struct FCamera
{
    static constexpr float DEFAULT_SCALE = 60.0f;
    float Scale = DEFAULT_SCALE;
};

union FObjectSubtype
{
    ENpcType NpcType = ENpcType::SomeGuy;
    EOneOffEffectType OneOffEffectType;
    EEnvironmentType EnvironmentType;
};

class TObject
{
public:

    static TObject* Get(FObjectRef Ref);

    static TObject* TryGet(FObjectRef& Ref, bool bAllowNullify=true);

    static bool IsValid(FObjectRef Ref);

    void Init();

    void Release();

    inline bool HasFlags(EObjectFlags CheckFlags) const
    {
        return (Flags & CheckFlags) != EObjectFlags::None;
    }

    inline bool HasFlag(EObjectFlags CheckFlag) const
    {
        return HasFlags(CheckFlag);
    }

    inline bool CanEverMove() const
    {
        return HasFlags(EObjectFlags::MoveFromInput | EObjectFlags::MoveAsAttachment);
    }

    TVector2 GetPosition();

    inline FGraphic* PrimaryGraphic() {  return Graphics.Count() > 0 ? &Graphics[0] : nullptr; }

    void AttachToParent(FObjectRef ParentRef, TVector2 Offset={});

    void DetachFromParent();

    void DetachChildren();

    // should use this instead of Get() or TryGet(), because it auto-detaches if the parent is invalid
    TObject* TryGetParent();

public:

    FObjectRef Self;
    FObjectRef HeldObject;
    TDynamicArray<FObjectRef> Attachments;
    FObjectRef AttachParent;
    TVector2 Position;
    s32 BaseDepth;
    TDynamicArray<FGraphic> Graphics;
    FObjectMovement Movement;
    EObjectFlags Flags;
    EDirection Orientation;
    EOverlapChannel OverlapChannels;
    ECollisionChannel CollisionChannels;

    // instead of comptime subclassing, this is a dogpile approach where the runtime-identified type 
    // identifies which data the object makes use of and how it updates. good for (game-making) iteration speed.
    FCharacter Character;
    FNpc Npc;
    FCamera Camera;
    FObjectSubtype SubType;
};

