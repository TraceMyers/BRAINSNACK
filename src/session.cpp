#include <stdio.h>

#include <emscripten.h>

#include "core.h"
#include "session.h"
#include "globals.h"

void TSession::Init()
{
    BeginTime = CurrentTime();        
    TimeSeconds = CurrentTime();
    _DeltaTime = CLAMP(DELTA_TIME_MAX * 0.3f, DELTA_TIME_MIN, DELTA_TIME_MAX);
    LevelExtent = {20, 20};

    InitAllocators();
    InitObjects();

    RecentFrameTimes.Init(RECENT_FRAME_TIME_COUNT, &RuntimeAllocator);
}

void TSession::Shutdown()
{
    ShutdownAllocators();
}

void TSession::BeginFrame()
{
}

void TSession::EndFrame()
{
    float64 NewTimeSeconds = CurrentTime();
    _DeltaTime = CLAMP(NewTimeSeconds - TimeSeconds, DELTA_TIME_MIN, DELTA_TIME_MAX);
    TimeSeconds = NewTimeSeconds;

    RecentFrameTimes.PushFront(_DeltaTime);
    if (bLogFps)
    {
        LogFpsTimer -= _DeltaTime;
        if (LogFpsTimer <= 0)
        {
            LogFpsTimer += LOG_FPS_WAIT_TIME;
            LOG("FPS: %", CalcSmoothedFPS());
        }
    }

    if (Mode == ESessionMode::TransitionToPlayGame && Renderer.FinishedTransitionToGame())
    {
        Mode = ESessionMode::PlayGame;
        TimeStartedPlay = TimeSeconds;
    }

    FrameCount++;

    FrameAllocator.Reset(true);

    if (bQuit)
    {
        LOG("quitting...");
        emscripten_cancel_main_loop();
    }
}

float64 TSession::ProgressiveTime()
{
    return CurrentTime() - BeginTime;
}

TObject *TSession::SpawnObject(EObjectType Type, TVector2 Position, FObjectRef OldRef, FObjectSubtype SubType)
{
    const s16 i = (s16)Objects.RequestItem();
    const s32 ID = ObjectIDCounter++;
    TObject* Object = &Objects[i];

    FObjectRef NewRef = {Type, i, ID};

    if (NewRef.ID != INVALID_OBJECT_ID)
    {
        // mapping old ref to new ref so that references can be fixed post-load
        Session.LoadedObjects_OldRefToNewRef.Set(OldRef, NewRef);
    }

    *Object = {};
    Object->Self = NewRef;
    Object->Position = Position;
    Object->SubType = SubType;
    Object->Init();

    // if (Goober == nullptr)
    // {
    //     // todo: place entity into grid
    // }

    return Object;
}

// todo: this should probably just be the release queuing proc. awkward overlap with Object->Release()
void TSession::ReleaseObject(FObjectRef& ID)
{
    TObject* Object = TObject::Get(ID);
    Object->Release();

    if (Object->HasFlag(EObjectFlags::PlacedInGrid))
    {
        // todo: remove entity from grid
    }

    Object->Attachments.Free();
    Object->Graphics.Free();
    Objects.ReturnItem(ID.Index);
}

void TSession::Quit()
{
    LOG("session received quit input");
    bQuit = true;
}

bool TSession::Quitting()
{
    return bQuit;
}

float32 TSession::CalcSmoothedFrameTime() const
{
    TDynamicArray<float32> FrameTimeDump;
    FrameTimeDump.TempInit(RecentFrameTimes.Count());
    RecentFrameTimes.DumpFrontToBack(FrameTimeDump);

    // value in (0, 1)
    // lesser decay = more smooth
    // higher decay = more sensitive to change
    constexpr float32 WEIGHT_DECAY = 0.03f;
    constexpr float32 WEIGHT_DESTAY = 1.0f - WEIGHT_DECAY; // heh de-stay. higher = moreso stays the same

    float32 WeightedFrameTimeSum = 0;
    float32 RunningWeight = 1.0f;
    float32 WeightSum = 0;

    for (int i = 0; i < FrameTimeDump.Count(); i++)
    {
        WeightedFrameTimeSum += FrameTimeDump[i] * RunningWeight;
        WeightSum += RunningWeight;
        RunningWeight *= WEIGHT_DESTAY;
    }

    const float WeightedAvgFps = WeightedFrameTimeSum / WeightSum;
    return WeightedAvgFps;

}

float32 TSession::CalcSmoothedFPS() const
{
    return 1.0f / CalcSmoothedFrameTime();
}

// ---------------------------------------------------------------------------------------------------------------------

void TSession::InitAllocators()
{
    constexpr u32 RUNTIME_ALLOCATION_SIZE   = ONE_MB * 40;
    constexpr u32 FRAME_ALLOCATION_SIZE     = ONE_MB * 10;
    constexpr u32 MAIN_ALLOCATION_SIZE      = ONE_MB * 20;
    constexpr s32 OBJECT_COUNT_MAX          = 32768;

    // allocating large block from the OS. the rest of the program's allocations will come from this block.
    RuntimeAllocator.Init(RUNTIME_ALLOCATION_SIZE, CStdAllocator);

    FrameAllocator.Init(FRAME_ALLOCATION_SIZE, RuntimeAllocator);
    MainAllocator.Init(MAIN_ALLOCATION_SIZE, RuntimeAllocator);
    // Objects will use the defualt allocator during SetCapacity
    DefaultAllocator = &RuntimeAllocator;
    Objects.SetCapacity(OBJECT_COUNT_MAX);

    // don't allow these allocators to reallocate. that would be unsafe because pointers into their old blocks will become invalid,
    // or unsound, as in the case of having a bump allocator as a parent allocator (realloc = massively wasteful).
    // also, the program's allocations are easy to put an upper bound on.
    RuntimeAllocator.SetParent(&NoAllocator, false);
    FrameAllocator.SetParent(&NoAllocator, false);
    MainAllocator.SetParent(&NoAllocator, false);
    Objects.LockAllocation();

    // used by arrays if you don't want to pass in a specific allocator
    // todo: after implement super allocator, make it the default
    DefaultAllocator = &CStdAllocator;
}

void TSession::ShutdownAllocators()
{
    RuntimeAllocator.Reset(false);
    FrameAllocator.Reset(false);
}

void TSession::InitObjects()
{
    InitializeAiBehaviorCdfs();
    
    TObject* PlayerObj = SpawnObject(EObjectType::Player);
    Session.PlayerID = PlayerObj->Self;

    TObject* Camera = SpawnObject(EObjectType::Camera);
    Session.MainCameraID = Camera->Self;
    Camera->AttachToParent(PlayerObj->Self);

    TObject* TeleportDest = SpawnObject(EObjectType::Teleporter);
    Session.TeleporterDestID = TeleportDest->Self;

    FObjectSubtype SubType;
    SubType.NpcType = ENpcType::SomeGuy;
    for (int i = 0; i < 1; i++)
    {
        SpawnObject(EObjectType::NPC, {1,1}, {}, SubType);
    }

    // SubType.NpcType = ENpcType::Butterfly;
    // SpawnObject(EObjectType::NPC, {}, {}, SubType);
}

bool TSession::DebugSwitch(bool bDebugBreak)
{
    if (bBreakSwitch)
    {
        LOG("break switch triggered");
        if (bDebugBreak)
        {
            // pause execution here in web browser debugger
            emscripten_debugger();
        }
        bBreakSwitch = false;
        return true;
    }
    else
    {
        return false;
    }
}
