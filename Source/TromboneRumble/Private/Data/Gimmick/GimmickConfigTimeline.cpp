// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Data/Gimmick/BeerFloodGimmickConfig.h"
#include "Data/Gimmick/BlackHoleGimmickConfig.h"
#include "Data/Gimmick/BonusGimmickConfig.h"
#include "Data/Gimmick/DrunkardGimmickConfig.h"
#include "Data/Gimmick/GarbageGimmickConfig.h"
#include "Data/Gimmick/GravityGimmickConfig.h"
#include "Data/Gimmick/SpotlightGimmickConfig.h"
#include "Data/Gimmick/UfoGimmickConfig.h"
#include "Data/Gimmick/WaterDropGimmickConfig.h"

#if WITH_EDITOR
#include "Data/Gimmick/GimmickTimeline.h"

// Each function repeats the timer rule of its gimmick actor. The actor is named above the function

// AGimmickManager runs the event, and each gimmick lists its phases in GetEventPhases
float UEventGimmickConfig::BuildEventTimeline(FGimmickTimelineBuilder& Builder, const float Start) const
{
	float Time = Start;
	if (bHasWarning)
	{
		Time = Builder.AddSpan(Time, WarningDuration, EGimmickTimelinePhase::Warning, FText::FromString(TEXT("예고")));
	}

	TArray<FGimmickEventPhase> Phases;
	GetEventPhases(Builder.GetWorld(), Phases);
	for (const FGimmickEventPhase& Phase : Phases)
	{
		Time = Builder.AddSpan(Time, Phase.Duration, Phase.Phase, Phase.Label);
	}
	return Time;
}

float UUfoGimmickConfig::BuildEventTimeline(FGimmickTimelineBuilder& Builder, const float Start) const
{
	Builder.SetNote(FText::FromString(TEXT("광선은 이동 라인 길이만큼 켜집니다. 레벨 이동 라인들의 평균 길이로 도착, 광선 펼침, 이동, 광선 접힘, 퇴장을 이어서 그리고, 라인이 없으면 이동을 5초로 둡니다")));
	return Super::BuildEventTimeline(Builder, Start);
}

float UDrunkardGimmickConfig::BuildEventTimeline(FGimmickTimelineBuilder& Builder, const float Start) const
{
	Builder.SetNote(FText::FromString(TEXT("아무도 잡히지 않아 지속시간을 끝까지 쓰고, 퇴장도 제한 시간을 끝까지 쓴 경우입니다. 포획되거나 문에 일찍 닿으면 그만큼 앞당겨집니다")));
	return Super::BuildEventTimeline(Builder, Start);
}

// AGimmickSpawnerBase. Every spawner shares this loop, and only the spans of one spawn differ
void USpawnerGimmickConfig::BuildTimeline(FGimmickTimelineBuilder& Builder) const
{
	float BusyUntil = 0.f;
	for (float SpawnTime = FirstSpawnDelay; Builder.IsInRound(SpawnTime);)
	{
		// The spawner skips a spawn that comes while the last one still keeps it busy
		if (SpawnTime >= BusyUntil)
		{
			BusyUntil = BuildSpawnTimeline(Builder, SpawnTime);
		}

		// The wait runs from the spawn, so objects overlap when the interval is shorter than their life
		const bool bFever = bUseFeverRule && Builder.IsFever(SpawnTime);
		const FGimmickInterval& Interval = GetSpawnRule(bFever).Interval;
		SpawnTime += FMath::Max(Builder.Pick(Interval.Min, Interval.Max), FGimmickTimelineBuilder::MinStep);
	}
}

float USpawnerGimmickConfig::BuildSpawnTimeline(FGimmickTimelineBuilder& Builder, const float SpawnTime) const
{
	Builder.AddSpan(SpawnTime, 0.f, EGimmickTimelinePhase::Spawn, FText::FromString(TEXT("스폰")));
	return SpawnTime;
}

// ASpotlightZone
float USpotlightGimmickConfig::BuildSpawnTimeline(FGimmickTimelineBuilder& Builder, const float SpawnTime) const
{
	Builder.SetNote(FText::FromString(TEXT("한 번의 스폰을 한 묶음으로 그립니다. 한 번에 여러 곳에 생기는 개수는 표시하지 않습니다")));

	float Time = Builder.AddSpan(SpawnTime, WarningDuration, EGimmickTimelinePhase::Warning, FText::FromString(TEXT("예고")));
	Time = Builder.AddSpan(Time, ActiveDuration, EGimmickTimelinePhase::Active, FText::FromString(TEXT("발동")));
	Builder.AddSpan(Time, FadeDuration, EGimmickTimelinePhase::Ending, FText::FromString(TEXT("사라짐")));
	return SpawnTime;
}

// AGarbageSpawner
float UGarbageGimmickConfig::BuildSpawnTimeline(FGimmickTimelineBuilder& Builder, const float SpawnTime) const
{
	Builder.AddSpan(SpawnTime, 0.f, EGimmickTimelinePhase::Spawn, FText::FromString(TEXT("투척")));
	return SpawnTime;
}

// ABonusSpawner
float UBonusGimmickConfig::BuildSpawnTimeline(FGimmickTimelineBuilder& Builder, const float SpawnTime) const
{
	Builder.SetNote(FText::FromString(TEXT("떨어지는 시간은 빼고 그립니다. 빈 지점이 모자라 적게 떨어지는 경우는 표시하지 않습니다")));

	float DropTime = SpawnTime;
	if (WarningDuration > 0.f)
	{
		DropTime = Builder.AddSpan(SpawnTime, WarningDuration, EGimmickTimelinePhase::Warning, FText::FromString(TEXT("예고")));
	}

	if (Lifetime > 0.f)
	{
		Builder.AddSpan(DropTime, Lifetime, EGimmickTimelinePhase::Active, FText::FromString(TEXT("유지")));
	}
	else
	{
		Builder.AddSpan(DropTime, 0.f, EGimmickTimelinePhase::Spawn, FText::FromString(TEXT("낙하")));
	}

	// The spawner skips a spawn that comes while the warning is still up
	return DropTime;
}

// AWaterDropSpawner and APuddleTrap
float UWaterDropGimmickConfig::BuildSpawnTimeline(FGimmickTimelineBuilder& Builder, const float SpawnTime) const
{
	// The ice shares this rule, so the labels take the name of the mark from the type
	const bool bIce = GetGimmickType() == EGimmickType::IceDrop;
	const FText MarkLabel = FText::FromString(bIce ? TEXT("빙판") : TEXT("물자국"));
	const FText FadeLabel = FText::FromString(bIce ? TEXT("빙판 사라짐") : TEXT("물자국 사라짐"));

	Builder.SetNote(FText::Format(FText::FromString(TEXT("물방울이 떨어지는 시간은 빼고, 생성 시점부터 {0}을 그립니다")), MarkLabel));

	const float FadeStart = Builder.AddSpan(SpawnTime, GrowDuration + HoldDuration, EGimmickTimelinePhase::Active, MarkLabel);
	Builder.AddSpan(FadeStart, FadeDuration, EGimmickTimelinePhase::Ending, FadeLabel);
	return SpawnTime;
}
#endif
