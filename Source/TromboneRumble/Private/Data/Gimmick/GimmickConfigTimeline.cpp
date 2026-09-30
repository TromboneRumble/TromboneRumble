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
#include "Actors/Gimmick/Ufo/UfoGimmick.h"
#include "Data/Gimmick/GimmickTimeline.h"
#include "EngineUtils.h"

// Each function repeats the timer rule of its gimmick actor. The actor is named above the function

// ABeerFloodGimmick
float UBeerFloodGimmickConfig::BuildEventTimeline(FGimmickTimelineBuilder& Builder, const float Start) const
{
	float Time = Builder.AddSpan(Start, WarningDuration, EGimmickTimelinePhase::Warning, FText::FromString(TEXT("전조")));
	Time = Builder.AddSpan(Time, RisingDuration, EGimmickTimelinePhase::Active, FText::FromString(TEXT("수위 상승")));
	Time = Builder.AddSpan(Time, SustainDuration, EGimmickTimelinePhase::Active, FText::FromString(TEXT("침수 유지")));
	return Builder.AddSpan(Time, DrainingDuration, EGimmickTimelinePhase::Ending, FText::FromString(TEXT("배수")));
}

// AGravityGimmick
float UGravityGimmickConfig::BuildEventTimeline(FGimmickTimelineBuilder& Builder, const float Start) const
{
	const float Time = Builder.AddSpan(Start, WarningDuration, EGimmickTimelinePhase::Warning, FText::FromString(TEXT("예고")));
	return Builder.AddSpan(Time, ActiveDuration, EGimmickTimelinePhase::Active, FText::FromString(TEXT("중력 변경")));
}

// ABlackHoleGimmick
float UBlackHoleGimmickConfig::BuildEventTimeline(FGimmickTimelineBuilder& Builder, const float Start) const
{
	// The black hole grows, then collapses, and both count as the event
	float Time = Builder.AddSpan(Start, WarningDuration, EGimmickTimelinePhase::Warning, FText::FromString(TEXT("예고")));
	Time = Builder.AddSpan(Time, ActiveDuration, EGimmickTimelinePhase::Active, FText::FromString(TEXT("성장")));
	return Builder.AddSpan(Time, CollapseDuration, EGimmickTimelinePhase::Ending, FText::FromString(TEXT("붕괴")));
}

// AUfoGimmick and AUfo
float UUfoGimmickConfig::BuildEventTimeline(FGimmickTimelineBuilder& Builder, const float Start) const
{
	// The beam stays on as long as the line is, and only the gimmick actor in the level knows the lines
	float LineLength = 0.f;
	if (const UWorld* World = Builder.GetWorld())
	{
		for (TActorIterator<AUfoGimmick> It(World); It; ++It)
		{
			LineLength = It->GetAverageLineLength();
			break;
		}
	}

	constexpr float FallbackLineSeconds = 5.f;
	const float LineSeconds = LineLength > 0.f ? LineLength / FMath::Max(MoveSpeed, 1.f) : FallbackLineSeconds;
	Builder.SetNote(FText::FromString(LineLength > 0.f
		? TEXT("광선은 라인 길이만큼 켜집니다. 레벨 이동 라인들의 평균 길이로 도착, 광선 펼침, 이동, 광선 접힘, 퇴장을 이어서 그립니다")
		: TEXT("레벨에 이동 라인이 없어 라인 이동을 5초로 두고 도착, 광선 펼침, 이동, 광선 접힘, 퇴장을 이어서 그립니다")));
	const float EventDuration = GetIntroDuration() + LineSeconds + BeamDeployDuration + WarpDuration;

	const float Time = Builder.AddSpan(Start, WarningDuration, EGimmickTimelinePhase::Warning, FText::FromString(TEXT("예고")));
	return Builder.AddSpan(Time, EventDuration, EGimmickTimelinePhase::Active, FText::FromString(TEXT("광선")));
}

// ADrunkardSpawner and UDrunkardStateComponent
float UDrunkardGimmickConfig::BuildEventTimeline(FGimmickTimelineBuilder& Builder, const float Start) const
{
	Builder.SetNote(FText::FromString(TEXT("아무도 잡히지 않아 지속시간을 끝까지 쓰고, 퇴장도 제한 시간을 끝까지 쓴 경우입니다. 포획되거나 문에 일찍 닿으면 그만큼 앞당겨집니다")));

	float Time = Builder.AddSpan(Start, EnterBurstDuration + EnterDuration, EGimmickTimelinePhase::Warning, FText::FromString(TEXT("등장")));
	Time = Builder.AddSpan(Time, ChaseDuration, EGimmickTimelinePhase::Active, FText::FromString(TEXT("추격 (최대)")));
	return Builder.AddSpan(Time, ExitTimeout, EGimmickTimelinePhase::Ending, FText::FromString(TEXT("퇴장 (최대)")));
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

// ABonusSpawner
void UBonusGimmickConfig::BuildTimeline(FGimmickTimelineBuilder& Builder) const
{
	// 0 stops the spawner
	if (SpawnInterval <= 0.f) return;

	Builder.SetNote(FText::FromString(TEXT("떨어지는 시간은 빼고 그립니다. 빈 지점이 모자라 적게 떨어지거나 건너뛰는 경우는 표시하지 않습니다")));

	const float Step = FMath::Max(SpawnInterval, FGimmickTimelineBuilder::MinStep);
	float WarningEnd = 0.f;
	for (float StartTime = Step; Builder.IsInRound(StartTime); StartTime += Step)
	{
		// The spawner skips a round that starts while the last warning is still up
		if (StartTime < WarningEnd) continue;

		float DropTime = StartTime;
		if (WarningDuration > 0.f)
		{
			DropTime = Builder.AddSpan(StartTime, WarningDuration, EGimmickTimelinePhase::Warning, FText::FromString(TEXT("예고")));
		}
		WarningEnd = DropTime;

		if (Lifetime > 0.f)
		{
			Builder.AddSpan(DropTime, Lifetime, EGimmickTimelinePhase::Active, FText::FromString(TEXT("유지")));
		}
		else
		{
			Builder.AddSpan(DropTime, 0.f, EGimmickTimelinePhase::Spawn, FText::FromString(TEXT("낙하")));
		}
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
