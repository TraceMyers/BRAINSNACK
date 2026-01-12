#pragma once

#include "core.h"
#include "modifier.h"

struct TItem
{
    TString Name;
    TArray<TModifier> Modifiers;
};

struct TWeapon
{
    TString Name;
    TArray<TModifier> Modifiers;
};