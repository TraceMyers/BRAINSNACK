#pragma once

#include "../math.h"

class TObject;

class TObjectMovement
{
public:

    void FrameUpdate(float DeltaTime);

protected:

    bool MoveDueToInput(TObject* Object, TVector2& OutMoveDelta, float DeltaTime);

    bool MoveDueToPhysics(TObject* Object, TVector2& OutMoveDelta, float DeltaTime);

    void MoveDueToAttachment(TObject* Object, float DeltaTime);

public:

    bool bSkipUpdate;

};