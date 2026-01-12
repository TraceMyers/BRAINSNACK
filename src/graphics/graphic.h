#pragma once

#include "../fundamental_defs.h"
#include "../math.h"
#include "color.h"

class TObject;

typedef s32 AnimFrameInt;

// every animated sprite in the game has a 24-frame strip
inline constexpr AnimFrameInt ANIMATED_SPRITE_FRAME_COUNT = 24;

extern const TVector2 SPRITE_TEXTURE_DIMENSIONS;
extern const TVector2 SPRITE_CELL_DIMENSIONS;
extern const TVector2 SPRITE_TEXTURE_CELL_COUNTS;
extern const TVector2 INV_SPRITE_TEXTURE_CELL_COUNTS;
extern const TVector2 SPRITE_CELLS_LOWER_RIGHT_CORNER;

// an FGraphic's data represents
enum class EGraphicType : u8
{
    // just a quad
    ColorQuad, 
    // fixed frame count sprite strip
    AnimatedSprite, 
    // single, potentially large UV, image
    StaticSprite 
};

// FGraphic's coordinate system
enum class EViewSpace : u8
{
    World,
    Window
};

struct FQuad 
{
    TVector2 UpperLeft;
    TVector2 LowerRight;
};

struct FAnimatedSprite
{
    static constexpr float32 DEFAULT_ANIM_SPEED_WHEN_IDLE = 2;
    static constexpr float32 DEFAULT_ANIM_SPEED_WHEN_MOVING = 6;
    TVector2 UpperLeftUv;
    TVector2 LowerRightUv;
    // current frame
    AnimFrameInt AnimFrame; 
    // lower bound for current animation
    AnimFrameInt AnimFrameMin; 
    // upper bound for current animation
    AnimFrameInt AnimFrameMax; 
    // how much progress has been made toward advancing the frame
    float32 AnimAdvanceProgress; 
    float32 AnimAdvanceSpeed = 1;
    float32 AnimAdvanceSpeedWhenIdle = DEFAULT_ANIM_SPEED_WHEN_IDLE;
    float32 AnimAdvanceSpeedWhenMoving = DEFAULT_ANIM_SPEED_WHEN_MOVING;
    s32 TotalFrameCount = 1;
};

struct FStaticSprite
{
    TVector2 UpperLeftUv;
    TVector2 LowerRightUv;
};

struct FGraphic
{
    TVector2 Offset;
    TVector2 Extent = {0.5f, 0.5f};
    s32 RenderDepth;
    EViewSpace Space = EViewSpace::World;
    EGraphicType Type = EGraphicType::ColorQuad;
    // graphics are used as colliders for simplicity's sake, so overlaps and collisions are per-graphic
    bool bDoesOverlap;
    // graphics are used as colliders for simplicity's sake, so overlaps and collisions are per-graphic
    bool bDoesCollide;
    bool bVisible = true;
    bool bHiddenDuringPlay;
    union 
    {
        Color4f Color = {};
        FAnimatedSprite AnimatedSprite; 
        FStaticSprite StaticSprite;
    };
};

inline s32 GraphicSortProc(const FGraphic *A, const FGraphic *B)
{
    // return A->Offset.Y - B->Offset.Y; // todo: why doesn't this work?
    if (A->Offset.Y < B->Offset.Y)
    {
        return -1;
    }
    else if (A->Offset.Y > B->Offset.Y)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

enum class ECharacterAnimFrame : AnimFrameInt
{
    IdleDownBegin       = 0,
    IdleDownEnd         = 1,
    IdleUpBegin         = 2,
    IdleUpEnd           = 3,
    IdleRightBegin      = 4,
    IdleRightEnd        = 5,
    IdleLeftBegin       = 6,
    IdleLeftEnd         = 7,
    WalkDownBegin       = 8,
    WalkDownEnd         = 11,
    WalkUpBegin         = 12,
    WalkUpEnd           = 15,
    WalkRightBegin      = 16,
    WalkRightEnd        = 19,
    WalkLeftBegin       = 20,
    WalkLeftEnd         = 23
};

// the sprites within the sprite sheet could potentially change in pixel size, but coords 1,1, will always refer to
// the upper left corner of the sprite at row 1, column 1, and the lower right rorner of the sprite at row 0, col 0
struct FTextureCellSelector
{
    TVector2 UpperLeft;
    TVector2 LowerRight; 
};

// convert a TextureCellSelector point to a UV coordinate in the sprite sheet
inline TVector2 SpriteCellToUV(TVector2 Point)
{
    return Point * INV_SPRITE_TEXTURE_CELL_COUNTS;
}

// maybe not ideal, but assumes the graphic belongs to the object. initializes the graphic to be an animated
// sprite, assuming the relationship. todo: rethink
void SetAnimatedSprite(const TObject* Object, FGraphic& Graphic);

// get the sprite texture selector for text
FTextureCellSelector GetTextureCellSelector(const s8 Ascii);

// get the sprite texture selector for this object's main graphic
FTextureCellSelector GetTextureCellSelector(const TObject* Object);