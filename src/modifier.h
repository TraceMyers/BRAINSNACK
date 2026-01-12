#pragma once

class TObject;

#include "fundamental_defs.h"

enum class EModifierType
{
    AddBaseDamage,
    AddMeleeDamage,
    AddRangedDamage,
    AddFireDamage,
    AddPiercingDamage,
    AddBluntDamage,
    AddSlowingAttack,
    ScaleBaseDamage,
    ScaleCriticalHitChance,
    ScaleCriticalHit,
    ScaleDodge,
    ScaleSpeed,
    AddRange,
    AddAttackSpeed,
    ScaleAttackSpeed,
    AddArmor,   
    AddMaxHP,
    AddHPOnce,
    AddHPRegen,
    AddPickupRange,
    AddEnemyHP,
    ScaleEnemyHP,
    AddKnockback,
    AddGravitation,
    ScaleSpeedPerKill,
    ScaleHPPerKill,
    ScaleStunChance,
    ScaleStunDuration,
    ScaleEnemyCount,
    ScaleMoneyPerKill,
    ScaleXPGain,
    ScaleFireSpread,
    AddProjectilePenetration
};

class TModifier
{
public:

    void Apply(TObject* Object);

public:

    EModifierType Type;
    float32 Value;
};

enum class EStatusType
{
    OnFire,
    Bleeding,
    Slowed,
    Stunned
};

class TStatus
{
public:

    void Apply(TObject* Object);

public:

    EStatusType Type;
    float32 MainValue;
    float32 SecondaryValue;
    float32 TimeRemaining;
    float32 TimeInitial;
};