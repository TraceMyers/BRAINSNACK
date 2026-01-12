#pragma once

#include "../fundamental_defs.h"
#include "../string.h"
#include "../allocators/array.h"
#include "../math.h"

namespace Mesh
{
    // pretty basic version of an obj loader. just loads vertex positions because it's used for a specific purpose (render brain)
    bool LoadObj(const TString& Path, TDynamicArray<TVector3>& OutVerts);
}