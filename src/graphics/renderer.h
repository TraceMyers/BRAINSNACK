#pragma once

#include "../fundamental_defs.h"
#include "../math.h"
#include "../string.h"
#include "graphic.h"
#include "../allocators/array.h"

class SDL_Texture;
class SDL_Window;
class SDL_Renderer;

struct TVertex
{
    TVector2 Position;
    Color4f Color;
    TVector2 UV;
};

enum class EHzTextAlign
{
    Left,
    Center,
    Right
};

enum class EVtTextAlign
{
    Top,
    Center,
    Bottom
};

enum class ETextSize
{
    Small,
    Medium,
    Large
};

class TRenderer
{
public:

    void Init();

    void Shutdown();

    void RenderFrame(float32 DeltaTime);

    void RenderMenu(float32 DeltaTime);

    void RenderGame(float32 DeltaTime, float32 TransitionAlpha);

    void RenderBrain(float32 DeltaTime, TVector2 ScreenLocation, float32 Scale);

    inline TVector2 WindowExtent() const { return TVector2((float)WindowWidth, (float)WindowHeight) * 0.5f; }

    bool OverlapsWindow(const FQuad& Quad);

    SDL_Texture* LoadTexture(const s8* Path) const;

    void UpdateGraphics(TObject* Object, TDynamicArray<FGraphic>& OutDraws, float DeltaTime);

    void OrientSprite(FAnimatedSprite& Spr, EDirection Orientation, bool bIsMoving);

    bool AnimateSprite(FAnimatedSprite& Spr, const TObject* Object, float DeltaTime);

    void DrawQuad(const FQuad& Quad, TVector2 UvUpperLeft={0,0}, TVector2 UvLowerRight={1,1}, Color4f Color=Color4f::White(), SDL_Texture* Texture=nullptr);

    void DrawAsciiCharacter(s8 Ascii, const FQuad& Quad, Color4f Color=Color4f::White());

    void DrawText(TString Str, TVector2 ScreenPos, float32 CharHeight, float32 CharSpacing, EHzTextAlign Align=EHzTextAlign::Left, EVtTextAlign VtAlign=EVtTextAlign::Top, Color4f Color=Color4f::White(), float32 LineWidth=FLOAT32_MAX, float32 LineSpacing=0);

    void DrawText(TString Str, TVector2 ScreenPos, ETextSize Size, EHzTextAlign Align=EHzTextAlign::Left, EVtTextAlign VtAlign=EVtTextAlign::Top,  Color4f Color=Color4f::White(), float32 LineWidth=FLOAT32_MAX, float32 LineSpacing=0);

    void FlushDraws();

    void DrawWindowCrosshairs(Color4f Color);

    FQuad GetPlayGameButtonQuad();

    bool FinishedTransitionToGame() { return TransitionToGameAlpha == 1; }

protected:

    void DrawAnimatedSprite(const FGraphic& Graphic);

    void DrawGraphic(const FGraphic& Graphic);

    TVector2 BrainLocationInMenu()
    {
        // center of window
        return WindowExtent();
    }

    TVector2 BrainLocationInGame()
    {
        const TVector2 WindowCenter = WindowExtent();
        return {WindowCenter.X, WindowCenter.Y * 0.95f};
    }

    float32 BrainScaleInMenu()
    {
        return WindowExtent().Y * BrainScale * 2.8f;
    }

    float32 BrainScaleInGame()
    {
        return BrainScaleInMenu() * 0.05f;
    }

public:

    static constexpr s32 VERTEX_BATCH_MAX = 10000;
    static constexpr s32 DEFAULT_WINDOW_HEIGHT = 600;
    static constexpr float32 ASPECT_RATIO = 1920.0f / 1080.0f;
    static constexpr s32 DEFAULT_WINDOW_WIDTH = (s32)(DEFAULT_WINDOW_HEIGHT * ASPECT_RATIO);

    static constexpr float32 TEXT_SIZE_LARGE = 48;
    static constexpr float32 TEXT_SIZE_MEDIUM = 32;
    static constexpr float32 TEXT_SIZE_SMALL = 24;
    static constexpr float32 TEXT_SPACING_LARGE = -2;
    static constexpr float32 TEXT_SPACING_MEDIUM = -1;
    static constexpr float32 TEXT_SPACING_SMALL = 0;

    static constexpr float32 PLAY_GAME_BLINK_TIME = 0.5f;

    static constexpr float32 BRAIN_LINE_STEP_TURNOVER_TIME = 2.0f;

    static constexpr float32 TRANSITION_TO_GAME_SPEED = 0.2f;

    const Color4b RenderClearColor = {100, 0, 0, 255};

    bool bTrimText = true;
    float32 PlayGameBlinkTimer = PLAY_GAME_BLINK_TIME;

    u32 ClearColor = 0;
    Color4f PointColor;

    SDL_Window* Window;
    SDL_Renderer* SdlRenderer;
    SDL_Texture* Sprites;

    s32 WindowWidth;
    s32 WindowHeight;

    TString DriverName;

    TDynamicArray<TVertex> VertexBatch;
    TDynamicArray<TVector2> PointBatch;

    SDL_Texture* BatchTexture;

    TDynamicArray<TVector3> BrainVerts;

    float32 BrainScale;
    float32 InMenuBrainRotation = 0;

    float32 BrainLineStepTimer = BRAIN_LINE_STEP_TURNOVER_TIME;
    s32 BrainLineStepBegin = 0;
    s32 BrainLineStepSize = 32;

    Color4f BrainDotColor = Color4f::White();

    float32 TransitionToGameAlpha = 0;
    
    float32 BrainTest = 1;
};