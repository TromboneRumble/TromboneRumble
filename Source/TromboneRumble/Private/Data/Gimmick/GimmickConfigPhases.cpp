// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Actors/Gimmick/Ufo/UfoGimmick.h"
#include "Data/Gimmick/BeerFloodGimmickConfig.h"
#include "Data/Gimmick/BlackHoleGimmickConfig.h"
#include "Data/Gimmick/DrunkardGimmickConfig.h"
#include "Data/Gimmick/GravityGimmickConfig.h"
#include "Data/Gimmick/UfoGimmickConfig.h"
#include "EngineUtils.h"

// Each function follows the timer rule of its gimmick actor. The actor is named above the function

// AGravityGimmick
void UGravityGimmickConfig::GetEventPhases(const UWorld* World, TArray<FGimmickEventPhase>& OutPhases) const
{
	OutPhases.Add({ EGimmickTimelinePhase::Active, FText::FromString(TEXT("중력 변경")), ActiveDuration });
}

// ABlackHoleGimmick. The hole grows, then collapses, and both count as the event
void UBlackHoleGimmickConfig::GetEventPhases(const UWorld* World, TArray<FGimmickEventPhase>& OutPhases) const
{
	OutPhases.Add({ EGimmickTimelinePhase::Active, FText::FromString(TEXT("성장")), GrowDuration });
	OutPhases.Add({ EGimmickTimelinePhase::Ending, FText::FromString(TEXT("붕괴")), CollapseDuration });
}

// ABeerFloodGimmick
void UBeerFloodGimmickConfig::GetEventPhases(const UWorld* World, TArray<FGimmickEventPhase>& OutPhases) const
{
	OutPhases.Add({ EGimmickTimelinePhase::Active, FText::FromString(TEXT("수위 상승")), RisingDuration });
	OutPhases.Add({ EGimmickTimelinePhase::Active, FText::FromString(TEXT("침수 유지")), SustainDuration });
	OutPhases.Add({ EGimmickTimelinePhase::Ending, FText::FromString(TEXT("배수")), DrainingDuration });
}

// AUfoGimmick and AUfo
void UUfoGimmickConfig::GetEventPhases(const UWorld* World, TArray<FGimmickEventPhase>& OutPhases) const
{
	// The beam stays on as long as the line is, and only the gimmick actor in the level knows the lines
	float LineLength = 0.f;
	if (World)
	{
		for (TActorIterator<AUfoGimmick> It(World); It; ++It)
		{
			LineLength = It->GetAverageLineLength();
			break;
		}
	}

	constexpr float FallbackLineSeconds = 5.f;
	const float LineSeconds = LineLength > 0.f ? LineLength / FMath::Max(MoveSpeed, 1.f) : FallbackLineSeconds;

	// Arrival and beam spread, the line, the beam fold, then the windup and the warp out
	const float BeamDuration = GetIntroDuration() + LineSeconds + BeamDeployDuration + WarpWindupDuration + WarpDuration;
	OutPhases.Add({ EGimmickTimelinePhase::Active, FText::FromString(TEXT("광선")), BeamDuration });
}

// ADrunkardSpawner and UDrunkardStateComponent. The chase and the exit are their longest, as the drunkard can end them early
void UDrunkardGimmickConfig::GetEventPhases(const UWorld* World, TArray<FGimmickEventPhase>& OutPhases) const
{
	OutPhases.Add({ EGimmickTimelinePhase::Warning, FText::FromString(TEXT("등장")), DoorPassDuration + EnterPauseDuration });
	OutPhases.Add({ EGimmickTimelinePhase::Active, FText::FromString(TEXT("추격 (최대)")), ChaseDuration });
	OutPhases.Add({ EGimmickTimelinePhase::Ending, FText::FromString(TEXT("퇴장 (최대)")), ExitTimeout });
}
