#pragma once

#include "core.h"

// some debug/testing code I might leave around for the public repo. I usually prefer to just tests inline, the delete, so
// it may not end up having much in it.
namespace RunTest {

    // just checking I translated the hashing code correctly.
    // note: takes a minute or so, and requires clicking 'wait' repeatedly to tell the browser to wait to become responsive
    inline void GetHash()
    {
        constexpr s32 BYTE_COUNT = 12;
        constexpr s32 ITERATION_COUNT = 100000;

        TDynamicArray<u32> Hashes;
        Hashes.Init(ITERATION_COUNT);

        u64 Bytes = 0;
        s32 CollisionCount = 0;

        for (s32 i = 0; i < ITERATION_COUNT; i++)
        {
            const u32 Hash = ::GetHash((s8*)&Bytes, 8);
            if (Hashes.Contains(Hash))
            {
                CollisionCount++;            
            }
            Hashes.Push(Hash);
            Bytes += 1;
        }

        LOG("after % iterations, found % collisions", ITERATION_COUNT, CollisionCount);
    }

    // some validation that the ToString() for enums is working. not much of a point to this, it was made
    // for a metaprogramming speedrun video. i'm keeping it around in case you happend to see that video :)
    inline void EnumStrings()
    {
        EObjectFlags Flag = EObjectFlags::SkipMoveUpdate;   
        EOneOffEffectType Effect = EOneOffEffectType::Turkey;
        ENpcState State = ENpcState::KeepDistanceFromPlayer;

        LOG("Flag: %", ToString(Flag));
        LOG("Effect: %", ToString(Effect));
        LOG("State: %", ToString(State));
    }

}
