// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Data/Gimmick/BeerFloodGimmickConfig.h"
#include "Data/Gimmick/DrunkardGimmickConfig.h"
#include "Data/Gimmick/GarbageGimmickConfig.h"
#include "Data/Gimmick/GravityGimmickConfig.h"
#include "Data/Gimmick/PresentGimmickConfig.h"
#include "Data/Gimmick/SpotlightGimmickConfig.h"
#include "Data/Gimmick/WaterDropGimmickConfig.h"

#if WITH_EDITOR
#include "Data/Gimmick/GimmickTimeline.h"

// Each function repeats the timer rule of its gimmick actor. The actor is named above the function

// ABeerFloodGimmick
void UBeerFloodGimmickConfig::BuildTimeline(FGimmickTimelineBuilder& Builder) const
{
	const float Step = FMath::Max3(Period, GetFloodDuration(), FGimmickTimelineBuilder::MinStep);

	for (float WarningStart = FirstWarningDelay; Builder.IsInRound(WarningStart); WarningStart += Step)
	{
		float Time = Builder.AddSpan(WarningStart, WarningDuration, EGimmickTimelinePhase::Warning, FText::FromString(TEXT("전조")));
		Time = Builder.AddSpan(Time, RisingDuration, EGimmickTimelinePhase::Active, FText::FromString(TEXT("수위 상승")));
		Time = Builder.AddSpan(Time, SustainDuration, EGimmickTimelinePhase::Active, FText::FromString(TEXT("침수 유지")));
		Builder.AddSpan(Time, DrainingDuration, EGimmickTimelinePhase::Ending, FText::FromString(TEXT("배수")));
	}
}

// AGravityGimmick
void UGravityGimmickConfig::BuildTimeline(FGimmickTimelineBuilder& Builder) const
{
	const auto PickInterval = [this, &Builder]() { return Builder.Pick(Schedule.IntervalMin, Schedule.IntervalMax); };

	float WarningStart = Schedule.FirstDelay >= 0.f ? Schedule.FirstDelay : PickInterval();
	while (Builder.IsInRound(WarningStart))
	{
		const float ActiveStart = Builder.AddSpan(WarningStart, Schedule.WarningDuration, EGimmickTimelinePhase::Warning, FText::FromString(TEXT("예고")));
		const float End = Builder.AddSpan(ActiveStart, ActiveDuration, EGimmickTimelinePhase::Active, FText::FromString(TEXT("중력 변경")));

		// The next wait starts when this event ends
		WarningStart = FMath::Max(End + PickInterval(), WarningStart + FGimmickTimelineBuilder::MinStep);
	}
}

// ADrunkardSpawner and UDrunkardStateComponent
void UDrunkardGimmickConfig::BuildTimeline(FGimmickTimelineBuilder& Builder) const
{
	Builder.SetNote(FText::FromString(TEXT("아무도 잡히지 않아 지속시간을 끝까지 쓰고, 퇴장도 제한 시간을 끝까지 쓴 경우입니다. 포획되거나 문에 일찍 닿으면 그만큼 앞당겨집니다")));

	float SpawnTime = InitialSpawnDelay;
	while (Builder.IsInRound(SpawnTime))
	{
		float Time = Builder.AddSpan(SpawnTime, EnterBurstDuration + EnterDuration, EGimmickTimelinePhase::Warning, FText::FromString(TEXT("등장")));
		Time = Builder.AddSpan(Time, ChaseDuration, EGimmickTimelinePhase::Active, FText::FromString(TEXT("추격 (최대)")));
		const float End = Builder.AddSpan(Time, ExitTimeout, EGimmickTimelinePhase::Ending, FText::FromString(TEXT("퇴장 (최대)")));

		// The respawn wait starts when the NPC is destroyed
		SpawnTime = FMath::Max(End + RespawnInterval, SpawnTime + FGimmickTimelineBuilder::MinStep);
	}
}

// ASpotlightManager and ASpotlightZone
void USpotlightGimmickConfig::BuildTimeline(FGimmickTimelineBuilder& Builder) const
{
	Builder.SetNote(FText::FromString(TEXT("존 하나를 한 묶음으로 그립니다. 한 번에 여러 곳에 생기는 개수는 표시하지 않습니다")));

	// The manager spawns the first zones as soon as it is activated
	float SpawnTime = 0.f;
	while (Builder.IsInRound(SpawnTime))
	{
		float Time = Builder.AddSpan(SpawnTime, WarningDuration, EGimmickTimelinePhase::Warning, FText::FromString(TEXT("경고")));
		Time = Builder.AddSpan(Time, ActiveDuration, EGimmickTimelinePhase::Active, FText::FromString(TEXT("활성")));
		Builder.AddSpan(Time, FadingDuration, EGimmickTimelinePhase::Ending, FText::FromString(TEXT("소멸")));

		// The wait runs from the spawn, so zones overlap when the interval is shorter than one zone
		const FGimmickInterval& Interval = GetRule(Builder.IsFever(SpawnTime)).Interval;
		SpawnTime += FMath::Max(Builder.Pick(Interval.Min, Interval.Max), FGimmickTimelineBuilder::MinStep);
	}
}

// AGarbageSpawner
void UGarbageGimmickConfig::BuildTimeline(FGimmickTimelineBuilder& Builder) const
{
	float SpawnTime = 0.f;
	while (true)
	{
		SpawnTime += FMath::Max(Builder.Pick(SpawnInterval.Min, SpawnInterval.Max), FGimmickTimelineBuilder::MinStep);
		if (!Builder.IsInRound(SpawnTime)) break;

		Builder.AddSpan(SpawnTime, 0.f, EGimmickTimelinePhase::Spawn, FText::FromString(TEXT("투척")));
	}
}

// APresentSpawner
void UPresentGimmickConfig::BuildTimeline(FGimmickTimelineBuilder& Builder) const
{
	// 0 stops the spawner
	if (SpawnInterval <= 0.f) return;

	const float Step = FMath::Max(SpawnInterval, FGimmickTimelineBuilder::MinStep);
	for (float SpawnTime = Step; Builder.IsInRound(SpawnTime); SpawnTime += Step)
	{
		Builder.AddSpan(SpawnTime, 0.f, EGimmickTimelinePhase::Spawn, FText::FromString(TEXT("선물 낙하")));
	}
}

// AWaterDropSpawner and APuddleTrap
void UWaterDropGimmickConfig::BuildTimeline(FGimmickTimelineBuilder& Builder) const
{
	// 0 stops the spawner
	if (SpawnInterval <= 0.f) return;

	Builder.SetNote(FText::FromString(TEXT("물방울이 떨어지는 시간은 빼고, 생성 시점부터 웅덩이를 그립니다")));

	const float Step = FMath::Max(SpawnInterval, FGimmickTimelineBuilder::MinStep);
	for (float SpawnTime = Step; Builder.IsInRound(SpawnTime); SpawnTime += Step)
	{
		const float FadeStart = Builder.AddSpan(SpawnTime, PuddleGrowDuration + PuddleFadeDelay, EGimmickTimelinePhase::Active, FText::FromString(TEXT("웅덩이")));
		Builder.AddSpan(FadeStart, PuddleFadeDuration, EGimmickTimelinePhase::Ending, FText::FromString(TEXT("웅덩이 소멸")));
	}
}
#endif
