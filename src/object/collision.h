#pragma once

#include "../core.h"

namespace Collision
{
    void ProcessPlayerCollisions(float DeltaTime);

    bool CellVisitor_CollectObjectsInCell(TVector2i CellXY, s32 CellIndex, void* UserData);
}