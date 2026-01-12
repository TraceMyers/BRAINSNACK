#pragma once

class TObject;

class TObjectMovement
{
public:

    void FrameUpdate(float DeltaTime);

protected:

    void MoveDueToInput(TObject* Object, float DeltaTime);

    void MoveDueToAttachment(TObject* Object, float DeltaTime);

public:

    bool bSkipUpdate;

};