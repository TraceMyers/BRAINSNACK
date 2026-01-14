#include <cassert>
#include <filesystem>

#include <SDL3/SDL.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "renderer.h"
#include "../core.h"
#include "mesh.h"

#define DRAW_POINT(Point, Color)                                                                                \
    do                                                                                                          \
    {                                                                                                           \
        const s32 NewPointCount = PointBatch.Count() + 1;                                                       \
        if (NewPointCount > PointBatch.Max() || Color != PointColor)                                            \
        {                                                                                                       \
            FlushDraws();                                                                                       \
        }                                                                                                       \
        PointColor = Color;                                                                                     \
        PointBatch.Push(Point);                                                                                 \
    }                                                                                                           \
    while (0);

#define DRAW_LINE(PointA, PointB, Color)                                                                        \
    do                                                                                                          \
    {                                                                                                           \
        const Color4b ByteColor = ToColor4b(Color);                                                             \
        SDL_SetRenderDrawColor(SdlRenderer, ByteColor.r, ByteColor.g, ByteColor.b, ByteColor.a);                \
        SDL_RenderLine(SdlRenderer, PointA.X, PointA.Y, PointB.X, PointB.Y);                                    \
    }                                                                                                           \
    while (0);

void TRenderer::Init()
{
    WindowWidth = DEFAULT_WINDOW_WIDTH;
    WindowHeight = DEFAULT_WINDOW_HEIGHT;

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    Window = SDL_CreateWindow("Trace's Cool Site", WindowWidth, WindowHeight, 0);

    if (Window == nullptr)
    {
        LOG_ERROR("failed to create sdl window");
        Session.Quit();
        return;
    }

    TDynamicArray<TString> PreferredDrivers;
    PreferredDrivers.TempInit(16);

    for (int i = 0; i < SDL_GetNumRenderDrivers(); i++)
    {
        TString Driver = TString::Temp(SDL_GetRenderDriver(i));
        LOG("found graphics driver: %", Driver.CStr());
        if (Driver.Contains("opengl"))
        {
            PreferredDrivers.Push(Driver);
        }
    }

    // todo: score the drivers. right now, anything gl is preferred
    DriverName = PreferredDrivers.Count() > 0 ? PreferredDrivers[0].Copy() : TString::New("software");
    SdlRenderer = SDL_CreateRenderer(Window, DriverName.CStr());

    LOG("selected graphics driver: %", DriverName.CStr());

    if (SdlRenderer == nullptr)
    {
        LOG_ERROR("failed to create renderer from driver");
        Session.Quit();
        return;
    }

    constexpr s32 VSYNC_EVERY_VERTICAL_REFRESH = 1;
    SDL_SetRenderVSync(SdlRenderer, VSYNC_EVERY_VERTICAL_REFRESH);

    Sprites = LoadTexture("assets/sprites.png");
    if (Sprites == nullptr)
    {
        Session.Quit();
        return;
    }
    SDL_SetTextureScaleMode(Sprites, SDL_SCALEMODE_NEAREST);

    // load the brain mesh asset 
    Mesh::LoadObj("assets/brain.obj", BrainVerts);

    // get the min and max xy's so that the brain can be scaled within the screen extents
    float32 MinX = FLOAT32_MAX;
    float32 MaxX = -FLOAT32_MAX;
    float32 MinY = FLOAT32_MAX;
    float32 MaxY = -FLOAT32_MAX;
    for (s32 i = 0; i < BrainVerts.Count(); i++)
    {
        MinX = MIN(MinX, BrainVerts[i].X);
        MaxX = MAX(MaxX, BrainVerts[i].X);
        MinY = MIN(MinY, BrainVerts[i].Y);
        MaxY = MAX(MaxY, BrainVerts[i].Y);
    }
    BrainScale = MIN(1.0f / (MaxX - MinX), 1.0f / (MaxY - MinY)) * 1.8f;

    VertexBatch.Init(VERTEX_BATCH_MAX);
    PointBatch.Init(VERTEX_BATCH_MAX);
}

void TRenderer::Shutdown()
{
    SDL_Quit();
}

void TRenderer::RenderFrame(float DeltaTime)
{
    SDL_SetRenderDrawColor(SdlRenderer, RenderClearColor.r, RenderClearColor.g, RenderClearColor.b, RenderClearColor.a);
    SDL_RenderClear(SdlRenderer);
    // -----------------------------------------------------------------------------------------------------------------
    // Draw!

    switch (Session.Mode)
    {
    case ESessionMode::Menu:
        RenderMenu(DeltaTime);
        break;
    case ESessionMode::TransitionToPlayGame:
        TransitionToGameAlpha = MIN(TransitionToGameAlpha + TRANSITION_TO_GAME_SPEED * DeltaTime, 1);
        RenderGame(DeltaTime, TransitionToGameAlpha);
        break;
    case ESessionMode::PlayGame:
        RenderGame(DeltaTime, 1);
        break;
    }

    // -----------------------------------------------------------------------------------------------------------------
    FlushDraws();
    SDL_RenderPresent(SdlRenderer);
}

void TRenderer::RenderMenu(float DeltaTime)
{
    RenderBrain(DeltaTime, BrainLocationInMenu(), BrainScaleInMenu());

    // --- render the text ---

    bTrimText = false;

    const Color4f TitleColor = Color4f::LightGrey();
    const TString TitleTextA = TString("BRAINSNACK");
    const Color4f PlayGameTextColor = ToColor4f(RenderClearColor);
    const Color4f PlayGameBoxColor = Color4f::White();
    const Color4f TitleBarColor = Color4f::Black();
    const TVector2 WindowCenter (WindowWidth / 2, WindowHeight / 2);

    DrawText(TitleTextA, WindowExtent(), ETextSize::Large, EHzTextAlign::Center, EVtTextAlign::Center, TitleColor, WindowWidth - 200, TEXT_SIZE_MEDIUM * 0.02f);

    const TString PlayGameStr = PlayGameBlinkTimer > 0 ? TString(">play game<") : TString(" play game ");
    PlayGameBlinkTimer -= DeltaTime;
    if (PlayGameBlinkTimer < -PLAY_GAME_BLINK_TIME)
    {
        PlayGameBlinkTimer = PLAY_GAME_BLINK_TIME;
    }

    FQuad PlayButtonQuad = GetPlayGameButtonQuad();
    const TVector2 PlayButtonCenter = PlayButtonQuad.UpperLeft + (PlayButtonQuad.LowerRight - PlayButtonQuad.UpperLeft) * 0.5f;
    DrawQuad(PlayButtonQuad, {}, {}, PlayGameBoxColor);

    DrawText(PlayGameStr, PlayButtonCenter, ETextSize::Medium, EHzTextAlign::Center, EVtTextAlign::Center, PlayGameTextColor, WindowWidth - 200, TEXT_SIZE_MEDIUM * 0.02f);
}

void TRenderer::RenderGame(float32 DeltaTime, float32 TransitionAlpha)
{
    bTrimText = true;

    TDynamicArray<FGraphic> GraphicsToDraw;
    const s32 ConservativeGraphicCountEstimate = (Session.Objects.TopIndex() + 1) * 2;
    GraphicsToDraw.TempInit(ConservativeGraphicCountEstimate);

    for (int i = 0; i <= Session.Objects.TopIndex(); i++)
    {
        if (!Session.Objects.IsItemInUse(i))
        {
            continue;
        }
        TObject* Obj = &Session.Objects[i];
        if (Obj->Graphics.Count() == 0)
        {
            continue;
        }
        UpdateGraphics(Obj, GraphicsToDraw, DeltaTime);
    }

    GraphicsToDraw.QuickSort(GraphicSortProc);
    
    // s-curve to transition the scale of all objects from menu to gameplay, sort of like zooming into the map from the menu
    // could use the camera zoom, but if the map has lots of stuff on it, that might put a ton of stuff in frame (and I'm not writing the world's best renderer here)
    // just do this every frame because it's not a perf concern. no need to add extra state.
    constexpr float32 INPUT_SCALE_FOR_TRANSITION = 8.0f;
    const float32 ExpInputForTransition = (TransitionAlpha * 2.0f * INPUT_SCALE_FOR_TRANSITION) - INPUT_SCALE_FOR_TRANSITION;
    float32 TransitionObjectScale = 1.0f / (1.0f + exp(-ExpInputForTransition));
    if (TransitionObjectScale > 0.99f)
    {
        TransitionObjectScale = 1.0f;
    }

    for (int i = 0; i < GraphicsToDraw.Count(); i++)
    {
        GraphicsToDraw[i].Extent *= TransitionObjectScale;
        DrawGraphic(GraphicsToDraw[i]);
    }
    FlushDraws(); // brain should draw over other stuff

    TVector2 BrainLocation = Lerp(BrainLocationInMenu(), BrainLocationInGame(), TransitionAlpha);
    const float32 HoverOffset = sin(Session.TimeSeconds*2.0f) * 8.0f * TransitionAlpha;
    BrainLocation.Y += HoverOffset;
    const float32 UseBrainScale = Lerp(BrainScaleInMenu(), BrainScaleInGame(), TransitionAlpha);
    RenderBrain(DeltaTime, BrainLocation, UseBrainScale);

    FlushDraws(); // text should draw over brain

    if (Session.TimeInGame() > 0 && Session.TimeInGame() < 3.0f)
    {
        const s32 DoubleTime = Session.TimeInGame() * 2.0f;
        if ((DoubleTime & 0x1) == 0)
        {
            DrawText("GO!", WindowExtent(), ETextSize::Medium, EHzTextAlign::Center, EVtTextAlign::Center);
        }
    }
    else if (Session.TimeInGame() > 5.0f)
    {
        // DrawText("Apologies that you probably wanted to play a cool game called BRAINSNACK. I'm still making it. Check back soon for updates :)", WindowExtent(), ETextSize::Medium, EHzTextAlign::Center, EVtTextAlign::Center, Color4f::White(), TEXT_SIZE_MEDIUM * 10.0f);
    }
}

void TRenderer::RenderBrain(float32 DeltaTime, TVector2 ScreenLocation, float32 Scale)
{
    BrainLineStepTimer -= DeltaTime;
    if (BrainLineStepTimer <= 0)
    {
        BrainLineStepTimer += BRAIN_LINE_STEP_TURNOVER_TIME;
        BrainLineStepBegin = IncrementWrap(BrainLineStepBegin, 0, BrainLineStepSize-1);
    }
    s32 BrainLineStep = BrainLineStepBegin;

    Color4f BrainLineColor;
    Color4f HighlightBrainLineColor;

    if (BrainTempColorTimer > 0)
    {
        BrainTempColorTimer -= DeltaTime;
        BrainLineColor = BrainTempColor;
        HighlightBrainLineColor = Brighten(BrainTempColor, 0.1f);
        BrainDotColor = Color4f::White();
    }
    else
    {
        BrainLineColor = Brighten(ToColor4f(RenderClearColor), 0.12f);
        HighlightBrainLineColor = Brighten(ToColor4f(RenderClearColor), 0.4f);
        BrainDotColor = Brighten(HighlightBrainLineColor, 0.2f);
    }

    float32 BrainRotation = 0;
    if (Session.Mode != ESessionMode::PlayGame)
    {
        constexpr float BRAIN_ROT_SPEED = 0.25f;
        InMenuBrainRotation += DeltaTime * BRAIN_ROT_SPEED;
        BrainRotation = InMenuBrainRotation;
    }
    else
    {
        TObject* PlayerObj = TObject::Get(Session.PlayerID);
        TVector2 FaceDir = ToVector2(PlayerObj->Orientation);
        const float32 CosTheta = FaceDir.X;
        const float32 SinTheta = FaceDir.Y;
        BrainRotation = acosf(CosTheta);
        if (SinTheta < 0)
        {
            BrainRotation = 2.0f * PI - BrainRotation;
        }
    }

    TMatrix M;
    M.SetYRotation(BrainRotation);

    for (s32 i = 0; i < BrainVerts.Count(); i++)
    {
        TVector3 Vec = BrainVerts[i] * Scale;
        // vertical flip. the dang brainstem is upward. todo: do before render
        Vec.Y *= -1.0f;
        // rotate 
        Vec = M.Mul(Vec);
        // todo: put translation into matrix
        TVector2 VertexPt = Vec.AsVector2() + ScreenLocation;

        DRAW_POINT(VertexPt, BrainDotColor)

        BrainLineStep = IncrementWrap(BrainLineStep, 0, BrainLineStepSize-1);
        if (BrainLineStep == 0)
        {
            DRAW_LINE(ScreenLocation, VertexPt, HighlightBrainLineColor);
        }
        else
        {
            DRAW_LINE(ScreenLocation, VertexPt, BrainLineColor);
        }
    }
}

bool TRenderer::OverlapsWindow(const FQuad &Quad)
{
    const bool bNoOverlap = Quad.UpperLeft.X > (float)WindowWidth || Quad.UpperLeft.Y > (float)WindowHeight || Quad.LowerRight.X < 0 || Quad.LowerRight.Y < 0;
    return !bNoOverlap;
}

SDL_Texture *TRenderer::LoadTexture(const s8 *Path) const
{
    s32 Width = 0; 
    s32 Height = 0;
    s32 Channels = 0;

    u8* Bitmap = stbi_load(Path, &Width, &Height, &Channels, 4);

    if (Bitmap == nullptr)
    {
        LOG_ERROR("failed to load bitmap %", Path);
        return nullptr;
    }
    if (Channels != 4)
    {
        stbi_image_free(Bitmap);
        LOG_ERROR("loading bitmap %, expected 4 channels, got %", Path, Channels);
        return nullptr;
    }

    const s32 RowWidth = Width * Channels;
    SDL_Surface* Surface = SDL_CreateSurfaceFrom(Width, Height, SDL_PIXELFORMAT_RGBA32, Bitmap, RowWidth);

    if (Surface == nullptr)
    {
        stbi_image_free(Bitmap);
        LOG_ERROR("failed to create surface from %", Path);
        return nullptr;
    }

    SDL_Texture* Texture = SDL_CreateTextureFromSurface(SdlRenderer, Surface);
    SDL_DestroySurface(Surface);
    stbi_image_free(Bitmap);

    if (Texture == nullptr)
    {
        LOG_ERROR("failed to create texture from surface for %", Path);
        return nullptr;
    }

    return Texture;
}

void TRenderer::UpdateGraphics(TObject *Object, TDynamicArray<FGraphic> &OutDraws, float DeltaTime)
{
    for (int i = 0; i < Object->Graphics.Count(); i++)
    {
        FGraphic& Graphic = Object->Graphics[i];

        if (!Graphic.bVisible)
        {
            continue;
        }

        if (Session.Mode == ESessionMode::PlayGame && Graphic.bHiddenDuringPlay)
        {
            continue;
        }

        if (Graphic.Type == EGraphicType::AnimatedSprite)
        {
            FAnimatedSprite& Spr = Graphic.AnimatedSprite;

            if (Object->CanEverMove())
            {
                // if the object finishes a move this frame, Movement.bMoving will be false. however,
                // the AnimateMovement flag will still be set, indicating that we should keep animating movement
                // at least one more frame. if object movement continues on the next frame, then we won't have 
                // interrupted it with an idle animation frame.
                const bool bAnimateMovement = Object->HasFlag(EObjectFlags::AnimateMovement);
                OrientSprite(Spr, Object->Orientation, bAnimateMovement);
                Spr.AnimAdvanceSpeed = bAnimateMovement ? Spr.AnimAdvanceSpeedWhenMoving : Spr.AnimAdvanceSpeedWhenIdle;
            }

            if (Spr.AnimAdvanceSpeed != 0)
            {
                const bool bDestroyObject = AnimateSprite(Spr, Object, DeltaTime);
                if (bDestroyObject)
                {
                    // todo: queue entity deletion
                    return;
                }
            }
        }

        FGraphic GraphicCopy = Graphic;
        GraphicCopy.Offset = Object->GetPosition();
        // GraphicCopy.RenderDepth += Object->BaseDepth;
        OutDraws.Push(GraphicCopy);
    }
}

void TRenderer::OrientSprite(FAnimatedSprite &Spr, EDirection Orientation, bool bIsMoving)
{
    if ((Orientation & EDirection::Down) != EDirection::None)
    {
        if (bIsMoving)
        {
            Spr.AnimFrameMin = (s32)ECharacterAnimFrame::WalkDownBegin;
            Spr.AnimFrameMax = (s32)ECharacterAnimFrame::WalkDownEnd;
        }
        else
        {
            Spr.AnimFrameMin = (s32)ECharacterAnimFrame::IdleDownBegin;
            Spr.AnimFrameMax = (s32)ECharacterAnimFrame::IdleDownEnd;
        }
    }
    else if ((Orientation & EDirection::Up) != EDirection::None)
    {
        if (bIsMoving)
        {
            Spr.AnimFrameMin = (s32)ECharacterAnimFrame::WalkUpBegin;
            Spr.AnimFrameMax = (s32)ECharacterAnimFrame::WalkUpEnd;
        }
        else
        {
            Spr.AnimFrameMin = (s32)ECharacterAnimFrame::IdleUpBegin;
            Spr.AnimFrameMax = (s32)ECharacterAnimFrame::IdleUpEnd;
        }
    }
    else if ((Orientation & EDirection::Right) != EDirection::None)
    {
        if (bIsMoving)
        {
            Spr.AnimFrameMin = (s32)ECharacterAnimFrame::WalkRightBegin;
            Spr.AnimFrameMax = (s32)ECharacterAnimFrame::WalkRightEnd;
        }
        else
        {
            Spr.AnimFrameMin = (s32)ECharacterAnimFrame::IdleRightBegin;
            Spr.AnimFrameMax = (s32)ECharacterAnimFrame::IdleRightEnd;
        }
    }
    else if ((Orientation & EDirection::Left) != EDirection::None)
    {
        if (bIsMoving)
        {
            Spr.AnimFrameMin = (s32)ECharacterAnimFrame::WalkLeftBegin;
            Spr.AnimFrameMax = (s32)ECharacterAnimFrame::WalkLeftEnd;
        }
        else
        {
            Spr.AnimFrameMin = (s32)ECharacterAnimFrame::IdleLeftBegin;
            Spr.AnimFrameMax = (s32)ECharacterAnimFrame::IdleLeftEnd;
        }
    }
    else
    {
        // LOG_ERROR("can't orient a sprite in the direction 'None'");
        // assert(false);
    }
    Spr.AnimFrame = CLAMP(Spr.AnimFrame, Spr.AnimFrameMin, Spr.AnimFrameMax);
}

bool TRenderer::AnimateSprite(FAnimatedSprite &Spr, const TObject* Object, float DeltaTime)
{
    Spr.AnimAdvanceProgress += Spr.AnimAdvanceSpeed * DeltaTime;
    const s32 AdvanceFrameCount = (s32)Spr.AnimAdvanceProgress;
    if (AdvanceFrameCount != 0)
    {
        const s32 PrevAnimFrame = Spr.AnimFrame;
        Spr.AnimFrame = AddWrap(Spr.AnimFrame, AdvanceFrameCount, Spr.AnimFrameMin, Spr.AnimFrameMax);
        Spr.AnimAdvanceProgress -= (float)AdvanceFrameCount;

        // one-off effects are destroyed when they reach the end of the animation (marked by looping back to the beginning)
        const bool bIsEffect = Object->Self.Type == EObjectType::OneOffEffect;
        const bool bIsGoober = Object->HasFlag(EObjectFlags::IsEditorGoober);
        if (PrevAnimFrame != Spr.AnimFrameMin && Spr.AnimFrame == Spr.AnimFrameMin && bIsEffect && !bIsGoober)
        {
            return true;
        }
    }
    return false;
}

void TRenderer::DrawQuad(const FQuad& Quad, TVector2 UvUpperLeft, TVector2 UvLowerRight, Color4f Color, SDL_Texture *Texture)
{
    const s32 NewVertexCount = VertexBatch.Count() + 6;
    if (Texture != BatchTexture || NewVertexCount > VertexBatch.Max())
    {
        FlushDraws();
    }

    const TVector2 UpperRight   = TVector2(Quad.LowerRight.X,   Quad.UpperLeft.Y);
    const TVector2 LowerLeft    = TVector2(Quad.UpperLeft.X,    Quad.LowerRight.Y);
    const TVector2 UvUpperRight = TVector2(UvLowerRight.X,      UvUpperLeft.Y);
    const TVector2 UvLowerLeft  = TVector2(UvUpperLeft.X,       UvLowerRight.Y);

    // upper right triangle 
    VertexBatch.Push({Quad.UpperLeft,       Color, UvUpperLeft});
    VertexBatch.Push({UpperRight,           Color, UvUpperRight});
    VertexBatch.Push({Quad.LowerRight,      Color, UvLowerRight});

    // lower left triangle 
    VertexBatch.Push({Quad.UpperLeft,       Color, UvUpperLeft});
    VertexBatch.Push({Quad.LowerRight,      Color, UvLowerRight});
    VertexBatch.Push({LowerLeft,            Color, UvLowerLeft});

    BatchTexture = Texture;
}

void TRenderer::DrawAsciiCharacter(s8 Ascii, const FQuad &Quad, Color4f Color)
{
    assert(Ascii >= 0);
    FTextureCellSelector Selector = GetTextureCellSelector(Ascii);
    const TVector2 UvUL = SpriteCellToUV(Selector.UpperLeft);
    const TVector2 UvLR = SpriteCellToUV(Selector.LowerRight);
    DrawQuad(Quad, UvUL, UvLR, Color, Sprites);
}

namespace
{
    inline void AlignRenderPosX(float32* RenderPosX, float32 LineWidth, EHzTextAlign Align)
    {
        if (Align == EHzTextAlign::Center)
        {
            *RenderPosX -= LineWidth * 0.5f;
        }
        else if (Align == EHzTextAlign::Right)
        {
            *RenderPosX -= LineWidth;
        }
    }

    class FTextPrep
    {
    public:
        TString Line;
        TString RemainingText;

        void ConsumeCharacter()
        {
            assert(RemainingText.Data != nullptr && RemainingText.Count > 0);
            if (Line.Data == nullptr)
            {
                Line.Data = RemainingText.Data;
                Line.Count = 0;
            }
            Line.Count++;
            RemainingText.Data++;
            RemainingText.Count--;
        }

        bool ReverseConsumeTo(s8* TextPoint)
        {
            if (TextPoint == nullptr)
            {
                return false;
            }

            const s32 ReversedLineCount = TextPoint - Line.Data;
            assert(ReversedLineCount < Line.Count);
            const s32 GivingCharsBack = Line.Count - ReversedLineCount;

            Line.Count = ReversedLineCount;
            RemainingText.Data -= GivingCharsBack;
            RemainingText.Count += GivingCharsBack;

            return true;
        }
    };

    inline void PrepareForWidthBoundedTextLine(FTextPrep* Prep, float32* RenderPosX, float32 MaxRenderWidth, float32 CharWidth, float32 SpaceWidth, EHzTextAlign Align, bool bTrimWhitespace)
    {
        // consume whitespace at the beginning
        if (bTrimWhitespace)
        {
            Prep->RemainingText = Prep->RemainingText.TrimLeft();
        }

        Prep->Line = {};
        if (Prep->RemainingText.Count == 0)
        {
            return;
        }

        Prep->ConsumeCharacter();

        s8* LastSpaceLocation = nullptr;
        float32 RenderWidthRemain = MaxRenderWidth - CharWidth;
        const float32 CharWidthWithSpace = CharWidth + SpaceWidth;

        while (Prep->RemainingText.Count > 0)
        {
            if (*Prep->RemainingText.Data == ' ')
            {
                LastSpaceLocation = Prep->RemainingText.Data;
            }
            if (RenderWidthRemain < CharWidth)
            {
                break;
            }
            RenderWidthRemain -= CharWidthWithSpace;
            Prep->ConsumeCharacter();
        }

        if (Prep->RemainingText.Count > 0)
        {
            // if the last character in the line or the first character of the next are not spaces,
            // we should try to find a space to break the line on
            if (Prep->Line[Prep->Line.Count-1] != ' ' && *Prep->RemainingText.Data != ' ')
            {
                Prep->ReverseConsumeTo(LastSpaceLocation);
            }
        }

        if (bTrimWhitespace)
        {
            Prep->Line = Prep->Line.TrimRight();
        }
        AlignRenderPosX(RenderPosX, Prep->Line.Count * CharWidth + (Prep->Line.Count-1) * SpaceWidth, Align);
    }
}

void TRenderer::DrawText(TString Str, TVector2 ScreenPos, float32 CharHeight, float32 CharSpacing, EHzTextAlign HzAlign, EVtTextAlign VtAlign, Color4f Color, float32 LineWidth, float32 LineSpacing)
{
    // ----------------

    struct FRenderTextLine
    {
        TString Line;
        TVector2 RenderPos;
    };

    // ----------------

    if (Str.Count == 0)
    {
        return;
    }

    const float32 CharWidth = CharHeight * 0.5f; 
    const float32 CharDelta = CharWidth + CharSpacing;

    TVector2 RenderPos = ScreenPos;
    TVector2 InitialRenderPos = RenderPos;

    FTextPrep TextPrep;
    TextPrep.Line = {};
    TextPrep.RemainingText = Str.Duplicate();

    TDynamicArray<FRenderTextLine> Lines;
    Lines.TempInit(32);

    const float32 VtRenderBegin = RenderPos.Y;
    while (true)
    {
        PrepareForWidthBoundedTextLine(&TextPrep, &RenderPos.X, LineWidth, CharWidth, CharSpacing, HzAlign, bTrimText);
        if (TextPrep.Line.Count == 0)
        {
            RenderPos.Y -= LineSpacing;
            break;
        }
        FRenderTextLine Line = {TextPrep.Line, RenderPos};
        Lines.Push(Line);
        RenderPos.X = InitialRenderPos.X;
        RenderPos.Y += CharHeight;
        RenderPos.Y += LineSpacing;
    }
    const float32 VtRenderEnd = RenderPos.Y;

    const float32 VtRenderHeight = (VtRenderEnd - VtRenderBegin);
    float32 VtRenderOffset = 0;
    if (VtAlign == EVtTextAlign::Center)
    {
        VtRenderOffset = -VtRenderHeight * 0.5f;
    }
    else if (VtAlign == EVtTextAlign::Bottom)
    {
        VtRenderOffset = -VtRenderHeight;
    }

    for (s32 i = 0; i < Lines.Count(); i++)
    {
        TString& Line = Lines[i].Line;
        RenderPos = Lines[i].RenderPos;
        RenderPos.Y += VtRenderOffset;
        for (s32 j = 0; j < Line.Count; j++)
        {
            if (Line[j] != ' ')
            {
                DrawAsciiCharacter(Line[j], {RenderPos, RenderPos + TVector2(CharWidth, CharHeight)}, Color);
            }
            RenderPos.X += CharDelta;
        }
    }
}

void TRenderer::DrawText(TString Str, TVector2 ScreenPos, ETextSize Size, EHzTextAlign HzAlign, EVtTextAlign VtAlign, Color4f Color, float32 LineWidth, float32 LineSpacing)
{
    switch (Size)
    {
    case ETextSize::Small:
        DrawText(Str, ScreenPos, TEXT_SIZE_SMALL, TEXT_SPACING_SMALL, HzAlign, VtAlign, Color, LineWidth, LineSpacing);
        break;
    case ETextSize::Medium:
        DrawText(Str, ScreenPos, TEXT_SIZE_MEDIUM, TEXT_SPACING_MEDIUM, HzAlign, VtAlign, Color, LineWidth, LineSpacing);
        break;
    case ETextSize::Large:
        DrawText(Str, ScreenPos, TEXT_SIZE_LARGE, TEXT_SPACING_LARGE, HzAlign, VtAlign, Color, LineWidth, LineSpacing);
        break;
    }
}

void TRenderer::FlushDraws()
{
    if (VertexBatch.Count() > 0)
    {
        SDL_RenderGeometry(
            SdlRenderer, 
            BatchTexture, 
            (SDL_Vertex*)VertexBatch.Data(), 
            VertexBatch.Count(), 
            nullptr, 
            0
        );
        VertexBatch.SetCount(0);
        BatchTexture = nullptr;
    }
    if (PointBatch.Count() > 0)
    {
        const Color4b PointByteColor = ToColor4b(PointColor);
        SDL_SetRenderDrawColor(SdlRenderer, PointByteColor.r, PointByteColor.g, PointByteColor.b, PointByteColor.a);
        SDL_RenderPoints(SdlRenderer, (SDL_FPoint*)PointBatch.Data(), PointBatch.Count());
        PointBatch.SetCount(0);
    }
}

void TRenderer::DrawWindowCrosshairs(Color4f Color)
{
    constexpr float32 LINE_EXTENT = 2;
    const TVector2 Center = WindowExtent();

    const TVector2 HzUL = {0, Center.Y-LINE_EXTENT};
    const TVector2 HzLR = {Center.X*2, Center.Y+LINE_EXTENT};

    const TVector2 VtUL = {Center.X-LINE_EXTENT, 0};
    const TVector2 VtLR = {Center.X+LINE_EXTENT, Center.Y*2};

    DrawQuad({HzUL, HzLR}, {}, {}, Color);
    DrawQuad({VtUL, VtLR}, {}, {}, Color);
}

FQuad TRenderer::GetPlayGameButtonQuad()
{
    if (Session.Mode == ESessionMode::Menu)
    {
        const TVector2 PlayTextCenter = WindowExtent() + TVector2(0, TEXT_SIZE_LARGE*1.05f);
        const TVector2 PlayButtonBoxUL = PlayTextCenter - TVector2(TEXT_SIZE_MEDIUM * 2.5f, TEXT_SIZE_MEDIUM*0.5f);
        const TVector2 PlayButtonBoxLR = PlayTextCenter + TVector2(TEXT_SIZE_MEDIUM * 2.5f, TEXT_SIZE_MEDIUM*0.5f);
        return {PlayButtonBoxUL, PlayButtonBoxLR};
    }
    return {};
}

void TRenderer::DrawAnimatedSprite(const FGraphic &Graphic)
{
    constexpr float FRAME_COUNT_INV = 1.0f / ANIMATED_SPRITE_FRAME_COUNT;

    assert(Graphic.Type == EGraphicType::AnimatedSprite);

    FQuad Quad = GetQuad(Graphic, {}, {1,1}, EViewSpace::Window);    
    if (!OverlapsWindow(Quad))
    {
        return;
    }

    const float YDiff = Graphic.AnimatedSprite.LowerRightUv.Y - Graphic.AnimatedSprite.UpperLeftUv.Y;
    const float UvUlY = Graphic.AnimatedSprite.UpperLeftUv.Y + YDiff * (float)(Graphic.AnimatedSprite.AnimFrame * FRAME_COUNT_INV);
    const float UvLrY = Graphic.AnimatedSprite.UpperLeftUv.Y + YDiff * (float)((Graphic.AnimatedSprite.AnimFrame+ 1) * FRAME_COUNT_INV);

    const TVector2 UvUpperLeft (Graphic.AnimatedSprite.UpperLeftUv.X, UvUlY);
    const TVector2 UvLowerRight (Graphic.AnimatedSprite.LowerRightUv.X, UvLrY);

    DrawQuad(Quad, UvUpperLeft, UvLowerRight, Color4f::White(), Sprites);
}

void TRenderer::DrawGraphic(const FGraphic &Graphic)
{
    if (Graphic.Type == EGraphicType::AnimatedSprite)
    {
        DrawAnimatedSprite(Graphic);
    }
}
