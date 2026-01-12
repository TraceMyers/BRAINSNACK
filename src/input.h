#pragma once

#include "fundamental_defs.h"
#include "math.h"

class TObject;

class TInput
{
public:

    void Init();

    void FrameUpdate(float DeltaTime);

    void TryDoMoveInput(TObject* Object, bool bInputval, EDirection MoveDir, float DeltaTime);

protected:

    void SystemInputUpdate(float DeltaTime);

    void ObjectControlUpdate(float DeltaTime);

    void NpcControlUpdate(TObject* Object, float DeltaTime);

    void PlayerControlUpdate(TObject* Object, float DeltaTime);

public:

    TVector2 MousePosition;

    bool bSkipControlUpdate;

    bool bUpInputPressed;
    bool bDownInputPressed;
    bool bLeftInputPressed;
    bool bRightInputPressed;

};