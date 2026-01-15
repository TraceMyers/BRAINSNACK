#pragma once

template <typename EnumType>
inline s32 EnumValueCount() { return 0; }
inline const char* ToString(EObjectType EnumVal)
{
    switch (EnumVal)
    {
    case EObjectType::None:
        return "None";
    case EObjectType::Player:
        return "Player";
    case EObjectType::Camera:
        return "Camera";
    case EObjectType::Environment:
        return "Environment";
    case EObjectType::NPC:
        return "NPC";
    case EObjectType::OneOffEffect:
        return "OneOffEffect";
    case EObjectType::Teleporter:
        return "Teleporter";
    }
}

template <>
inline s32 EnumValueCount<EObjectType>()
{
    return 7;
}

inline const char* ToString(EDirection EnumVal)
{
    switch (EnumVal)
    {
    case EDirection::None:
        return "None";
    case EDirection::Up:
        return "Up";
    case EDirection::Right:
        return "Right";
    case EDirection::Down:
        return "Down";
    case EDirection::Left:
        return "Left";
    }
}

template <>
inline s32 EnumValueCount<EDirection>()
{
    return 5;
}

inline const char* ToString(EModifierType EnumVal)
{
    switch (EnumVal)
    {
    case EModifierType::AddBaseDamage:
        return "AddBaseDamage";
    case EModifierType::AddMeleeDamage:
        return "AddMeleeDamage";
    case EModifierType::AddRangedDamage:
        return "AddRangedDamage";
    case EModifierType::AddFireDamage:
        return "AddFireDamage";
    case EModifierType::AddPiercingDamage:
        return "AddPiercingDamage";
    case EModifierType::AddBluntDamage:
        return "AddBluntDamage";
    case EModifierType::AddSlowingAttack:
        return "AddSlowingAttack";
    case EModifierType::ScaleBaseDamage:
        return "ScaleBaseDamage";
    case EModifierType::ScaleCriticalHitChance:
        return "ScaleCriticalHitChance";
    case EModifierType::ScaleCriticalHit:
        return "ScaleCriticalHit";
    case EModifierType::ScaleDodge:
        return "ScaleDodge";
    case EModifierType::ScaleSpeed:
        return "ScaleSpeed";
    case EModifierType::AddRange:
        return "AddRange";
    case EModifierType::AddAttackSpeed:
        return "AddAttackSpeed";
    case EModifierType::ScaleAttackSpeed:
        return "ScaleAttackSpeed";
    case EModifierType::AddArmor:
        return "AddArmor";
    case EModifierType::AddMaxHP:
        return "AddMaxHP";
    case EModifierType::AddHPOnce:
        return "AddHPOnce";
    case EModifierType::AddHPRegen:
        return "AddHPRegen";
    case EModifierType::AddPickupRange:
        return "AddPickupRange";
    case EModifierType::AddEnemyHP:
        return "AddEnemyHP";
    case EModifierType::ScaleEnemyHP:
        return "ScaleEnemyHP";
    case EModifierType::AddKnockback:
        return "AddKnockback";
    case EModifierType::AddGravitation:
        return "AddGravitation";
    case EModifierType::ScaleSpeedPerKill:
        return "ScaleSpeedPerKill";
    case EModifierType::ScaleHPPerKill:
        return "ScaleHPPerKill";
    case EModifierType::ScaleStunChance:
        return "ScaleStunChance";
    case EModifierType::ScaleStunDuration:
        return "ScaleStunDuration";
    case EModifierType::ScaleEnemyCount:
        return "ScaleEnemyCount";
    case EModifierType::ScaleMoneyPerKill:
        return "ScaleMoneyPerKill";
    case EModifierType::ScaleXPGain:
        return "ScaleXPGain";
    case EModifierType::ScaleFireSpread:
        return "ScaleFireSpread";
    case EModifierType::AddProjectilePenetration:
        return "AddProjectilePenetration";
    }
}

template <>
inline s32 EnumValueCount<EModifierType>()
{
    return 33;
}

inline const char* ToString(EStatusType EnumVal)
{
    switch (EnumVal)
    {
    case EStatusType::OnFire:
        return "OnFire";
    case EStatusType::Bleeding:
        return "Bleeding";
    case EStatusType::Slowed:
        return "Slowed";
    case EStatusType::Stunned:
        return "Stunned";
    }
}

template <>
inline s32 EnumValueCount<EStatusType>()
{
    return 4;
}

inline const char* ToString(EArgType EnumVal)
{
    switch (EnumVal)
    {
    case EArgType::U8:
        return "U8";
    case EArgType::U16:
        return "U16";
    case EArgType::U32:
        return "U32";
    case EArgType::U64:
        return "U64";
    case EArgType::S8:
        return "S8";
    case EArgType::S16:
        return "S16";
    case EArgType::S32:
        return "S32";
    case EArgType::S64:
        return "S64";
    case EArgType::Float32:
        return "Float32";
    case EArgType::Float64:
        return "Float64";
    case EArgType::Character:
        return "Character";
    case EArgType::Pointer:
        return "Pointer";
    case EArgType::CString:
        return "CString";
    case EArgType::TString:
        return "TString";
    case EArgType::TVector2:
        return "TVector2";
    }
}

template <>
inline s32 EnumValueCount<EArgType>()
{
    return 15;
}

inline const char* ToString(ESessionMode EnumVal)
{
    switch (EnumVal)
    {
    case ESessionMode::Menu:
        return "Menu";
    case ESessionMode::TransitionToPlayGame:
        return "TransitionToPlayGame";
    case ESessionMode::PlayGame:
        return "PlayGame";
    }
}

template <>
inline s32 EnumValueCount<ESessionMode>()
{
    return 3;
}

inline const char* ToString(EPoolResizeBehavior EnumVal)
{
    switch (EnumVal)
    {
    case EPoolResizeBehavior::Default:
        return "Default";
    case EPoolResizeBehavior::Allow:
        return "Allow";
    case EPoolResizeBehavior::Disallow:
        return "Disallow";
    }
}

template <>
inline s32 EnumValueCount<EPoolResizeBehavior>()
{
    return 3;
}

inline const char* ToString(EGraphicType EnumVal)
{
    switch (EnumVal)
    {
    case EGraphicType::ColorQuad:
        return "ColorQuad";
    case EGraphicType::AnimatedSprite:
        return "AnimatedSprite";
    case EGraphicType::StaticSprite:
        return "StaticSprite";
    }
}

template <>
inline s32 EnumValueCount<EGraphicType>()
{
    return 3;
}

inline const char* ToString(EViewSpace EnumVal)
{
    switch (EnumVal)
    {
    case EViewSpace::World:
        return "World";
    case EViewSpace::Window:
        return "Window";
    }
}

template <>
inline s32 EnumValueCount<EViewSpace>()
{
    return 2;
}

inline const char* ToString(ECharacterAnimFrame EnumVal)
{
    switch (EnumVal)
    {
    case ECharacterAnimFrame::IdleDownBegin:
        return "IdleDownBegin";
    case ECharacterAnimFrame::IdleDownEnd:
        return "IdleDownEnd";
    case ECharacterAnimFrame::IdleUpBegin:
        return "IdleUpBegin";
    case ECharacterAnimFrame::IdleUpEnd:
        return "IdleUpEnd";
    case ECharacterAnimFrame::IdleRightBegin:
        return "IdleRightBegin";
    case ECharacterAnimFrame::IdleRightEnd:
        return "IdleRightEnd";
    case ECharacterAnimFrame::IdleLeftBegin:
        return "IdleLeftBegin";
    case ECharacterAnimFrame::IdleLeftEnd:
        return "IdleLeftEnd";
    case ECharacterAnimFrame::WalkDownBegin:
        return "WalkDownBegin";
    case ECharacterAnimFrame::WalkDownEnd:
        return "WalkDownEnd";
    case ECharacterAnimFrame::WalkUpBegin:
        return "WalkUpBegin";
    case ECharacterAnimFrame::WalkUpEnd:
        return "WalkUpEnd";
    case ECharacterAnimFrame::WalkRightBegin:
        return "WalkRightBegin";
    case ECharacterAnimFrame::WalkRightEnd:
        return "WalkRightEnd";
    case ECharacterAnimFrame::WalkLeftBegin:
        return "WalkLeftBegin";
    case ECharacterAnimFrame::WalkLeftEnd:
        return "WalkLeftEnd";
    }
}

template <>
inline s32 EnumValueCount<ECharacterAnimFrame>()
{
    return 16;
}

inline const char* ToString(EHzTextAlign EnumVal)
{
    switch (EnumVal)
    {
    case EHzTextAlign::Left:
        return "Left";
    case EHzTextAlign::Center:
        return "Center";
    case EHzTextAlign::Right:
        return "Right";
    }
}

template <>
inline s32 EnumValueCount<EHzTextAlign>()
{
    return 3;
}

inline const char* ToString(EVtTextAlign EnumVal)
{
    switch (EnumVal)
    {
    case EVtTextAlign::Top:
        return "Top";
    case EVtTextAlign::Center:
        return "Center";
    case EVtTextAlign::Bottom:
        return "Bottom";
    }
}

template <>
inline s32 EnumValueCount<EVtTextAlign>()
{
    return 3;
}

inline const char* ToString(ETextSize EnumVal)
{
    switch (EnumVal)
    {
    case ETextSize::Small:
        return "Small";
    case ETextSize::Medium:
        return "Medium";
    case ETextSize::Large:
        return "Large";
    }
}

template <>
inline s32 EnumValueCount<ETextSize>()
{
    return 3;
}

inline const char* ToString(EObjectFlags EnumVal)
{
    switch (EnumVal)
    {
    case EObjectFlags::None:
        return "None";
    case EObjectFlags::PlacedInGrid:
        return "PlacedInGrid";
    case EObjectFlags::AnimateMovement:
        return "AnimateMovement";
    case EObjectFlags::JustStartedMove:
        return "JustStartedMove";
    case EObjectFlags::JustFinishedMove:
        return "JustFinishedMove";
    case EObjectFlags::MoveFromInput:
        return "MoveFromInput";
    case EObjectFlags::MoveAsAttachment:
        return "MoveAsAttachment";
    case EObjectFlags::SkipMoveUpdate:
        return "SkipMoveUpdate";
    case EObjectFlags::SkipAIUpdate:
        return "SkipAIUpdate";
    case EObjectFlags::IsEditorGoober:
        return "IsEditorGoober";
    case EObjectFlags::ReleaseWithoutDeath:
        return "ReleaseWithoutDeath";
    case EObjectFlags::IsEnemy:
        return "IsEnemy";
    }
}

template <>
inline s32 EnumValueCount<EObjectFlags>()
{
    return 12;
}

inline const char* ToString(EOverlapChannel EnumVal)
{
    switch (EnumVal)
    {
    case EOverlapChannel::None:
        return "None";
    case EOverlapChannel::Enemies:
        return "Enemies";
    }
}

template <>
inline s32 EnumValueCount<EOverlapChannel>()
{
    return 2;
}

inline const char* ToString(ECollisionChannel EnumVal)
{
    switch (EnumVal)
    {
    case ECollisionChannel::None:
        return "None";
    case ECollisionChannel::People:
        return "People";
    case ECollisionChannel::Enemies:
        return "Enemies";
    case ECollisionChannel::Ephemera:
        return "Ephemera";
    }
}

template <>
inline s32 EnumValueCount<ECollisionChannel>()
{
    return 4;
}

inline const char* ToString(ENpcType EnumVal)
{
    switch (EnumVal)
    {
    case ENpcType::SomeGuy:
        return "SomeGuy";
    }
}

template <>
inline s32 EnumValueCount<ENpcType>()
{
    return 1;
}

inline const char* ToString(EOneOffEffectType EnumVal)
{
    switch (EnumVal)
    {
    case EOneOffEffectType::Heart:
        return "Heart";
    case EOneOffEffectType::Snooze:
        return "Snooze";
    case EOneOffEffectType::Turkey:
        return "Turkey";
    case EOneOffEffectType::NetSwing:
        return "NetSwing";
    }
}

template <>
inline s32 EnumValueCount<EOneOffEffectType>()
{
    return 4;
}

inline const char* ToString(EEnvironmentType EnumVal)
{
    switch (EnumVal)
    {
    case EEnvironmentType::LongHouse:
        return "LongHouse";
    case EEnvironmentType::TreeA:
        return "TreeA";
    case EEnvironmentType::TreeB:
        return "TreeB";
    case EEnvironmentType::TreeC:
        return "TreeC";
    case EEnvironmentType::TreeD:
        return "TreeD";
    }
}

template <>
inline s32 EnumValueCount<EEnvironmentType>()
{
    return 5;
}

inline const char* ToString(ENpcState EnumVal)
{
    switch (EnumVal)
    {
    case ENpcState::None:
        return "None";
    case ENpcState::Pause:
        return "Pause";
    case ENpcState::RandomWalk:
        return "RandomWalk";
    case ENpcState::MoveTowardPlayer:
        return "MoveTowardPlayer";
    case ENpcState::KeepDistanceFromPlayer:
        return "KeepDistanceFromPlayer";
    }
}

template <>
inline s32 EnumValueCount<ENpcState>()
{
    return 5;
}

