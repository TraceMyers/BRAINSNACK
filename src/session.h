#pragma once

#include "allocators/allocator.h"
#include "allocators/cstd_allocator.h"
#include "allocators/super_allocator.h"
#include "allocators/bump_allocator.h"
#include "allocators/no_allocator.h"
#include "allocators/ring_buffer.h"
#include "allocators/pool.h"
#include "allocators/map.h"
#include "object/object.h"
#include "fundamental_defs.h"

class TObjectGrid;

enum class ESessionMode : u8
{
    Menu,
    TransitionToPlayGame,
    PlayGame
};

// fundamental singleton object for the game. keeps track of time and ...
class TSession
{
public:

    void Init();

    void Shutdown();

    void BeginFrame();

    void EndFrame();

    double ProgressiveTime();

    TObject* SpawnObject(EObjectType Type, TVector2 Position = {}, FObjectRef OldRef = {}, FObjectSubtype SubType = {});

    void ReleaseObject(FObjectRef& ID);

    void Quit();

    bool Quitting();

    float32 CalcSmoothedFrameTime() const;

    float32 CalcSmoothedFPS() const;

    float32 DeltaTime(bool bScaled=true) const { return _DeltaTime * DeltaTimeScale; }

    inline float32 TimeInGame()
    { 
        return Mode == ESessionMode::PlayGame ? MAX(TimeSeconds - TimeStartedPlay, 0.0f) : 0;
    }

    void QueueReturnObjectToPool(const TObject* Object);

    void InitAllocators();

    void ShutdownAllocators();

    void InitObjects();

    // this data is public for simplicity's sake. i find that privating most members on the most fundamental global 
    // object complicates the structure, and i'd rather leave the whole thing simpler by keeping everything public. 
    // trust!
public: 

    // don't have to account for div by 0 if it's just really small
    static constexpr float DELTA_TIME_MIN = 1e-6f;
    // limiting delta time avoids weird physics (and logic) bugs that assume the time between frames is reasonable. there are no real physics in this game, but it can't hurt.
    static constexpr float DELTA_TIME_MAX = 1.0f / 4.0f;
    static constexpr s32 RECENT_FRAME_TIME_COUNT = 120;

    float64 BeginTime = 0;
    float64 TimeStartedPlay = 0;
    float64 TimeSeconds = 0;
    s32 FrameCount = 0;

    ESessionMode Mode = ESessionMode::Menu;

    TCStdAllocator CStdAllocator;
    TBumpAllocator RuntimeAllocator;
    TBumpAllocator FrameAllocator;
    TSuperAllocator MainAllocator;
    TNoAllocator NoAllocator;
    TAllocator* DefaultAllocator;

    TPool<TObject> Objects;

    FObjectRef PlayerID;
    FObjectRef MainCameraID;
    FObjectRef TeleporterDestID;

    s32 ObjectIDCounter = INVALID_OBJECT_ID + 1;

    TRingBuffer<float32> RecentFrameTimes;

    TMap<FObjectRef> LoadedObjects_OldRefToNewRef;

    TDynamicArray<FObjectRef> QueuedObjectReleases;

    TObjectGrid* ObjectGrid;

    TVector2 LevelExtent;

protected:

    bool bQuit = false;

    float32 _DeltaTime = DELTA_TIME_MIN;
    float32 DeltaTimeScale = 1;

// debug stuff
public:

    static constexpr float LOG_FPS_WAIT_TIME = 1.0f;

    bool DebugSwitch(bool bDebugBreak=true);

    bool bLogFps = false;
    float LogFpsTimer = LOG_FPS_WAIT_TIME;
    bool bBreakSwitch = false;
    bool bWhatever = false;
};
