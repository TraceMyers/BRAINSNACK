#include "pool.h"
#include "../globals.h"

void PoolHelper::LockPoolAllocation(TPool<u8>* Pool)
{
    Pool->Items.SetAllocator(&Session.NoAllocator);
    Pool->InUse.BitBlocks.SetAllocator(&Session.NoAllocator);
    Pool->bDefaultAllowGrow = false;
    Pool->bDefaultAllowShrink = false;
}
