
#include "array.h"
#include "../globals.h"

TAllocator* GetDefaultAllocator()
{
    return Session.DefaultAllocator;
}

TAllocator* GetTempAllocator()
{
    return &Session.FrameAllocator;
}
