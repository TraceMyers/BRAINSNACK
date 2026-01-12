#include "ai.h"
#include "../core.h"

namespace NpcStateCdf
{
    TArray<TArray<FNpcStateCdfEntry>*> AllCdfs;
    TArray<FNpcStateCdfEntry> SomeGuy;
    // TArray<FNpcStateCdfEntry> Thinker;
    // TArray<FNpcStateCdfEntry> BusyBody;
    // TArray<FNpcStateCdfEntry> Flighty;
    // TArray<FNpcStateCdfEntry> Reader;
}

namespace
{
    // take a list of weighted random walk states and output a cumulative distribution function that can be
    // used to select a state with a uniformly randomly drawn value between 0 and 1.
    void MakeAiBehaviorCdf(TArray<FNpcStateCdfEntry>& Entries)
    {
        float32 WeightSum = 0;
        s32 FinalIndexWithChange = 0;

        for (s32 i = 0; i < Entries.Count(); i++)
        {
            WeightSum += Entries[i].Weight;
        }

        if (WeightSum == 0)
        {
            return;
        }

        const float InvWeightSum = 1.0f / WeightSum;
        float RunningCdfSum = 0;

        for (s32 i = 0; i < Entries.Count()-1; i++)
        {
            Entries[i].Weight *= InvWeightSum;
            Entries[i].Weight += RunningCdfSum;
            RunningCdfSum = Entries[i].Weight;
        }
        Entries[Entries.Count()-1].Weight = 1.0f;
    }
}

void InitializeAiBehaviorCdfs()
{
    TDynamicArray<FNpcStateCdfEntry> AiBuilder;
    AiBuilder.SetMax(32);    

    AiBuilder.Push({ENpcState::MoveTowardPlayer, 1.0f, 0.2f, 0.3f});
    NpcStateCdf::SomeGuy.Copy(AiBuilder);
    MakeAiBehaviorCdf(NpcStateCdf::SomeGuy);
    AiBuilder.SetCount(0);
}

void RandomDrawNpcState(TObject *Object)
{
    const float RandNorm = RandomZeroToOne();
    const TArray<FNpcStateCdfEntry>& Cdf = *Object->Npc.StateProbabilityCdf;

    TObject* PlayerObj = TObject::Get(Session.PlayerID);

    for (int i = 0; i < Cdf.Count(); i++)
    {
        if (RandNorm < Cdf[i].Weight)
        {
            Object->Npc.State = Cdf[i].State;
            Object->Npc.StateTimer = RandomFloat(Cdf[i].StateTimeMin, Cdf[i].StateTimeMax);
            Object->Movement.bMoving = false;
            switch (Object->Npc.State)
            {
            case ENpcState::Pause:
                break;
            case ENpcState::RandomWalk:
                {
                    Object->Movement.bMoving = true;
                    Object->Movement.Direction = RandomWalkDirectionCdfDraw(Object);
                }
                break;
            case ENpcState::MoveTowardPlayer:
                {
                    const TVector2 ToPlayerDiff = PlayerObj->GetPosition() - Object->GetPosition();
                    Object->Movement.bMoving = true;
                    Object->Movement.Direction = To8WayDirection(ToPlayerDiff);
                }
            case ENpcState::KeepDistanceFromPlayer:
                {
                    constexpr float32 MAX_ERROR = 0.5f;
                    const TVector2 ToPlayerDiff = PlayerObj->GetPosition() - Object->GetPosition();
                    const float32 ToPlayerDist = ToPlayerDiff.Magnitude();
                    const float32 TargetDist = Object->Npc.KeepDistanceFromPlayer;
                    const float32 MinDist = MAX(TargetDist - MAX_ERROR, 0);
                    const float32 MaxDist = TargetDist + MAX_ERROR;
                    if (ToPlayerDist < MinDist)
                    {
                        Object->Movement.bMoving = true;
                        Object->Movement.Direction = To8WayDirection(ToPlayerDiff * -1.0f);
                    }
                    else  if (ToPlayerDist > MaxDist)
                    {
                        Object->Movement.bMoving = true;
                        Object->Movement.Direction = To8WayDirection(ToPlayerDiff);
                    }
                }
            default:;
            }
            return;
        }
    }
    LOG_WARNING("improperly formed npc state cdf for object of type %", ToString(Object->Self.Type));
    Object->Npc.State = ENpcState::None;
}

EDirection RandomWalkDirectionCdfDraw(TObject *Object)
{
    constexpr s32 DIRECTION_COUNT = 4;
    constexpr s32 RANDOM_SAMPLE_COUNT = 3;
    constexpr EDirection UP_OR_DOWN = EDirection::Up | EDirection::Down;
    constexpr EDirection LEFT_OR_RIGHT = EDirection::Right | EDirection::Left;
    const EDirection Dir1 = RandomDirection();
    const float RandFloat = RandomZeroToOne();
    if (RandFloat > 0.5f)
    {
        const EDirection Dir2 = RandomDirection();
        if ((Dir1 & UP_OR_DOWN) != EDirection::None && (Dir2 & UP_OR_DOWN) != EDirection::None)
        {
            return Dir1;
        }
        else if ((Dir1 & LEFT_OR_RIGHT) != EDirection::None && (Dir2 & LEFT_OR_RIGHT) != EDirection::None)
        {
            return Dir1;
        }
        else
        {
            return Dir1 | Dir2;
        }
    }
    else
    {
        return Dir1;
    }
}

void InitMoveDirectionCdf(TObject *Object)
{
    float32* Cdf = Object->Npc.RandomWalk.MoveDirectionCdf;

    // initial weights per move direction
    Cdf[(s32)EDirection::Up] = RandomFloat(0.001f, 1.0f);
    Cdf[(s32)EDirection::Right] = RandomFloat(0.001f, 1.0f);
    // having the opposite directions be the normalized inverses makes for more biased movement, which looks 
    // more purposeful, rather than moving left, then right, then left, then right... though that can still happen
    Cdf[(s32)EDirection::Down] = 1.0f - Cdf[(s32)EDirection::Up];
    Cdf[(s32)EDirection::Left] = 1.0f - Cdf[(s32)EDirection::Right];

    constexpr float32 WEIGHT_SUM = 2.0f;
    constexpr float32 INV_SUM = 1.0f / WEIGHT_SUM;

    // convert the weights into a cdf
    Cdf[0] = MIN(Cdf[0] * INV_SUM,          1.0f);
    Cdf[1] = MIN(Cdf[0] + Cdf[1] * INV_SUM, 1.0f);
    Cdf[2] = MIN(Cdf[1] + Cdf[2] * INV_SUM, 1.0f);
    Cdf[3] = 1.0f;

    // cooldown for re-weighting
    Object->Npc.RandomWalk.MoveDirectionCdfReinitCooldown = RandomFloat(10.0f, 24.0f);
}
