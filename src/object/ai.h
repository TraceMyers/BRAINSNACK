#pragma once

#include "object.h"

namespace NpcStateCdf
{
    extern TArray<TArray<FNpcStateCdfEntry>*> AllCdfs;
    extern TArray<FNpcStateCdfEntry> SomeGuy;
}

inline s32 CompareCdfEntries(const FNpcStateCdfEntry* A, const FNpcStateCdfEntry* B)
{
    return A->Weight - B->Weight;
}

void InitializeAiBehaviorCdfs();

void RandomDrawNpcState(TObject* Object);

EDirection RandomWalkDirectionCdfDraw(TObject *Object);

void InitMoveDirectionCdf(TObject* Object);