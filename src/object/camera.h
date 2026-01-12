#pragma once

#include "object.h"
#include "../fundamental_defs.h"
#include "../math.h"
#include "../globals.h"

inline TVector2 WorldToWindowPoint(TVector2 WorldPosition)
{
    TObject* CameraObj = TObject::Get(Session.MainCameraID);
    const TVector2 CameraPosition = CameraObj->GetPosition();
    const TVector2 WindowExtent = Renderer.WindowExtent();
    const TVector2 CamRelativePosition = (WorldPosition - CameraPosition) * CameraObj->Camera.Scale;
    return CamRelativePosition + WindowExtent;
}

inline TVector2 WindowToWorldPoint(TVector2 WindowPosition)
{
    TObject* CameraObj = TObject::Get(Session.MainCameraID);
    const TVector2 CameraPosition = CameraObj->GetPosition();
    const TVector2 WindowExtent = Renderer.WindowExtent();   
    const TVector2 CamRelativePosition = WindowPosition - WindowExtent;
    return CamRelativePosition / CameraObj->Camera.Scale + CameraPosition;
}

inline TVector2 WindowToWorldVector(TVector2 Vector)
{
    TObject* CameraObj = TObject::Get(Session.MainCameraID);
    return Vector / CameraObj->Camera.Scale;
}

inline TVector2 WorldToWindowVector(TVector2 Vector)
{
    TObject* CameraObj = TObject::Get(Session.MainCameraID);
    return Vector * CameraObj->Camera.Scale;
}

inline float32 WindowToWorldScalar(float32 Scalar)
{
    TObject* CameraObj = TObject::Get(Session.MainCameraID);
    return Scalar / CameraObj->Camera.Scale;
}

inline float32 WorldToWindowScalar(float32 Scalar)
{
    TObject* CameraObj = TObject::Get(Session.MainCameraID);
    return Scalar * CameraObj->Camera.Scale;
}

inline TVector2 GetCameraUpperLeft()
{
    TObject* CameraObj = TObject::Get(Session.MainCameraID);
    const TVector2 CameraPosition = CameraObj->GetPosition();
    const TVector2 WindowExtent = Renderer.WindowExtent();
    return CameraPosition - WindowExtent / CameraObj->Camera.Scale;
}

inline TVector2 GetCameraLowerRight()
{
    TObject* CameraObj = TObject::Get(Session.MainCameraID);
    const TVector2 CameraPosition = CameraObj->GetPosition();
    const TVector2 WindowExtent = Renderer.WindowExtent();
    return CameraPosition + WindowExtent / CameraObj->Camera.Scale;
}

inline TVector2 ConvertSpace(TVector2 Point, EViewSpace InSpace, EViewSpace OutSpace)
{
    TVector2 OutPoint;
    if (InSpace == EViewSpace::World)
    {
        if (OutSpace == EViewSpace::World)
        {
            OutPoint = Point;
        }
        else
        {
            OutPoint = WorldToWindowPoint(Point);
        }
    }
    else
    {
        if (OutSpace == EViewSpace::World)
        {
            OutPoint = WindowToWorldPoint(Point);
        }
        else
        {
            OutPoint = Point;
        }
    }
    return OutPoint;
}

inline FQuad GetQuad(const FGraphic& Graphic, TVector2 BaseOffset={}, TVector2 ExtentScale={1,1}, EViewSpace OutSpace=EViewSpace::World)
{
    FQuad Quad = {
        ConvertSpace(BaseOffset + Graphic.Offset, Graphic.Space, OutSpace),
        ConvertSpace(BaseOffset + Graphic.Offset + Graphic.Extent * 2.0f * ExtentScale, Graphic.Space, OutSpace)
    };
    return Quad;
}