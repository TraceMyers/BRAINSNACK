#include "graphic.h"
#include "../object/object.h"
#include "../string.h"

const TVector2 SPRITE_TEXTURE_DIMENSIONS = {1024, 1024};
const TVector2 SPRITE_CELL_DIMENSIONS = {16, 16};
const TVector2 SPRITE_TEXTURE_CELL_COUNTS = SPRITE_TEXTURE_DIMENSIONS / SPRITE_CELL_DIMENSIONS;
const TVector2 INV_SPRITE_TEXTURE_CELL_COUNTS = {
    1.0f / SPRITE_TEXTURE_CELL_COUNTS.X,
    1.0f / SPRITE_TEXTURE_CELL_COUNTS.Y
};
const TVector2 SPRITE_CELLS_LOWER_RIGHT_CORNER = SPRITE_TEXTURE_DIMENSIONS / SPRITE_CELL_DIMENSIONS;

void SetAnimatedSprite(const TObject* Object, FGraphic &Graphic)
{
    Graphic.Type = EGraphicType::AnimatedSprite;
    Graphic.AnimatedSprite = {};
    Graphic.AnimatedSprite.AnimFrame    = (s32)ECharacterAnimFrame::IdleDownBegin;
    Graphic.AnimatedSprite.AnimFrameMin = (s32)ECharacterAnimFrame::IdleDownBegin;
    Graphic.AnimatedSprite.AnimFrameMax = (s32)ECharacterAnimFrame::IdleDownEnd;
    Graphic.AnimatedSprite.TotalFrameCount = ANIMATED_SPRITE_FRAME_COUNT;
    Graphic.AnimatedSprite.AnimAdvanceSpeed = Graphic.AnimatedSprite.AnimAdvanceSpeedWhenIdle;
    FTextureCellSelector Cells = GetTextureCellSelector(Object);
    Graphic.AnimatedSprite.UpperLeftUv = SpriteCellToUV(Cells.UpperLeft);
    Graphic.AnimatedSprite.LowerRightUv = SpriteCellToUV(Cells.LowerRight);
}

FTextureCellSelector GetTextureCellSelector(const s8 Ascii)
{
    assert(Ascii > 0);
    constexpr s32 FIRST_CODE_POINT_IN_FIRST_COLUMN = 33;
    constexpr s32 FIRST_CODE_POINT_IN_SECOND_COLUMN = 97;
    constexpr s32 LAST_CODE_POINT = 126;
    const TVector2 BeginCodePoint33 = TVector2(SPRITE_CELLS_LOWER_RIGHT_CORNER.X - 2, 0);
    const TVector2 BeginCodePoint97 = TVector2(SPRITE_CELLS_LOWER_RIGHT_CORNER.X - 1, 0);
    FTextureCellSelector Selector;
    if (Ascii < FIRST_CODE_POINT_IN_FIRST_COLUMN || Ascii > LAST_CODE_POINT)
    {
        constexpr s32 ASTERISK_CODE_POINT = 42;
        Selector.UpperLeft = BeginCodePoint33;
        Selector.UpperLeft.Y += ASTERISK_CODE_POINT - FIRST_CODE_POINT_IN_FIRST_COLUMN;
    }
    else if (Ascii < FIRST_CODE_POINT_IN_SECOND_COLUMN)
    {
        Selector.UpperLeft = BeginCodePoint33;
        Selector.UpperLeft.Y += Ascii - FIRST_CODE_POINT_IN_FIRST_COLUMN;
    }
    else
    {
        Selector.UpperLeft = BeginCodePoint97;
        Selector.UpperLeft.Y += Ascii - FIRST_CODE_POINT_IN_SECOND_COLUMN;
    }
    Selector.LowerRight = Selector.UpperLeft + TVector2(1, 1);
    return Selector;
}

FTextureCellSelector GetTextureCellSelector(const TObject *Object)
{
    switch (Object->Self.Type)
    {
    case EObjectType::Environment:
        switch (Object->SubType.EnvironmentType)
        {
        case EEnvironmentType::LongHouse:
            return {{0, 0}, {5, 2}};
        case EEnvironmentType::TreeA:
            return {{5, 0}, {6 ,1}};
        case EEnvironmentType::TreeB:
            return {{6, 0}, {7, 1}};
        case EEnvironmentType::TreeC:
            return {{5, 1}, {6, 2}};
        case EEnvironmentType::TreeD:
            return {{6, 1}, {7, 2}};
        default:
            assert(false);
        }
        break;
    case EObjectType::OneOffEffect:
        switch (Object->SubType.OneOffEffectType)
        {
        case EOneOffEffectType::Heart:
            return {{14, 0}, {15, 24}};
        case EOneOffEffectType::NetSwing:
            return {{7, 0}, {8, 24}};
        case EOneOffEffectType::Snooze:
            return {{8, 0}, {9, 24}};
        case EOneOffEffectType::Turkey:
            return {{12, 0}, {13, 24}};
        default:
            assert(false);
        }
        break;
    case EObjectType::Player:
        return {{10, 0}, {11, 24}};
    case EObjectType::NPC:
        switch (Object->SubType.NpcType)
        {
        // case ENpcType::Butterfly:
        //     return {{9, 0}, {10, 24}};
        case ENpcType::SomeGuy:
            return {{11, 0}, {12, 24}};
        }
    case EObjectType::Teleporter:
        return {{0, 2}, {1, 3}};
    // case EObjectType::ButterflyNet:
    //     return {{7, 0}, {8, 24}};
    default:
        assert(false);
    }
}

