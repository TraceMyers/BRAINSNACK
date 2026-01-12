#include <SDL3/SDL.h>
#include <SDL3/SDL_keycode.h>
#include <emscripten.h>
#include <emscripten/html5.h>

#include "input.h"
#include "globals.h"
#include "session.h"
#include "object/object.h"
#include "object/ai.h"

void TInput::Init()
{

}

void TInput::FrameUpdate(float DeltaTime)
{
    SystemInputUpdate(DeltaTime);
    ObjectControlUpdate(DeltaTime);
}

void TInput::SystemInputUpdate(float DeltaTime)
{
    SDL_Event InputEvent;

    SDL_GetMouseState(&MousePosition.X, &MousePosition.Y);

    while (SDL_PollEvent(&InputEvent) != 0)
    {
        if (Session.Mode == ESessionMode::Menu)
        {
            if (InputEvent.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
            {
                const FQuad PlayButtonArea = Renderer.GetPlayGameButtonQuad();
                if (IsPointInsideBox(MousePosition, PlayButtonArea.UpperLeft, PlayButtonArea.LowerRight))
                {
                    Session.Mode = ESessionMode::TransitionToPlayGame;
                }
            }
        }
        else if (Session.Mode == ESessionMode::TransitionToPlayGame)
        {
            // probably nothing should go here
        }
        else switch (InputEvent.type) // mode is PlayGame
        {
            // in native apps, 'x' button. not sure on browsers
            case SDL_EVENT_QUIT:
                Session.Quit();
                break;
            // key presses
            case SDL_EVENT_KEY_DOWN:
                switch (InputEvent.key.key) {
                case SDLK_ESCAPE:
                    Session.Quit();
                    break;
                case SDLK_0:
                    Session.bBreakSwitch = true;
                    break;
                case SDLK_W:
                case SDLK_UP:
                    bUpInputPressed = true;
                    break;
                case SDLK_D:
                case SDLK_RIGHT:
                    bRightInputPressed = true;
                    break;
                case SDLK_S:
                case SDLK_DOWN:
                    bDownInputPressed = true;
                    break;
                case SDLK_A:
                case SDLK_LEFT:
                    bLeftInputPressed = true;
                    break;
                default:;
                }
                break;
            case SDL_EVENT_KEY_UP:
                switch (InputEvent.key.key) {
                case SDLK_W:
                case SDLK_UP:
                    bUpInputPressed = false;
                    break;
                case SDLK_D:
                case SDLK_RIGHT:
                    bRightInputPressed = false;
                    break;
                case SDLK_S:
                case SDLK_DOWN:
                    bDownInputPressed = false;
                    break;
                case SDLK_A:
                case SDLK_LEFT:
                    bLeftInputPressed = false;
                    break;
                default:;
                }
                break;
            // mouse
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                break;
            case SDL_EVENT_MOUSE_MOTION:
                break;
            default:;
        }
    }
}

void TInput::ObjectControlUpdate(float DeltaTime)
{
    if (Session.Mode != ESessionMode::PlayGame)
    {
        return;
    }

    if (bSkipControlUpdate)
    {
        return;
    }

    for (int i = 0; i < Session.Objects.TopIndex()+1; i++)
    {
        if (!Session.Objects.IsItemInUse(i))
        {
            continue;
        }
        TObject* Object = &Session.Objects[i];
        if (!Object->CanEverMove())
        {
            continue;
        }
        if (Object->Npc.State == ENpcState::None)
        {
            continue;
        }
        NpcControlUpdate(Object, DeltaTime);
    }

    TObject* PlayerObj = TObject::Get(Session.PlayerID);
    PlayerControlUpdate(PlayerObj, DeltaTime);
}

void TInput::NpcControlUpdate(TObject *Object, float DeltaTime)
{
    if (Object->Self.Type != EObjectType::NPC)
    {
        return;
    }
    if (Object->HasFlag(EObjectFlags::SkipAIUpdate))
    {
        return;
    }

    Object->Npc.StateTimer -= DeltaTime;    
    if (Object->Npc.StateTimer <= 0)
    {
        RandomDrawNpcState(Object);
    }
}

void TInput::PlayerControlUpdate(TObject *PlayerObj, float DeltaTime)
{
    PlayerObj->Movement.bMoving = false;
    PlayerObj->Movement.Direction = EDirection::None;
    TryDoMoveInput(PlayerObj, bUpInputPressed,    EDirection::Up,     DeltaTime);
    TryDoMoveInput(PlayerObj, bRightInputPressed, EDirection::Right,  DeltaTime);
    TryDoMoveInput(PlayerObj, bDownInputPressed,  EDirection::Down,   DeltaTime);
    TryDoMoveInput(PlayerObj, bLeftInputPressed,  EDirection::Left,   DeltaTime);
}

void TInput::TryDoMoveInput(TObject *PlayerObj, bool bInputVal, EDirection MoveDir, float DeltaTime)
{
    if (bInputVal)
    {
        PlayerObj->Movement.bMoving = true;
        PlayerObj->Movement.Direction |= MoveDir;
    }
}
