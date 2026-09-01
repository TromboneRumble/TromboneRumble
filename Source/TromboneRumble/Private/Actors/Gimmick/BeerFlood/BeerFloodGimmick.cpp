// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Actors/Gimmick/BeerFlood/BeerFloodGimmick.h"
#include "Characters/TromboneCharacterBase.h"
#include "Components/ActorComponents/TromboneRagdollComponent.h"
#include "Components/CapsuleComponent.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"
#include "Utilities/EnumHelper.h"
#include "Utilities/TromboneLogs.h"
#include "Engine/Engine.h"

#if !UE_BUILD_SHIPPING
static TAutoConsoleVariable<int32> CVarBeerFloodDebug(
	TEXT("Trombone.BeerFlood.Debug"),
	0,
	TEXT("1이면 술통 침수 기믹의 단계와 수위를 화면과 월드에 표시"));
#endif

ABeerFloodGimmick::ABeerFloodGimmick()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	GimmickType = EGimmickType::BeerFlood;
}

void ABeerFloodGimmick::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, BeerFloodState);
	DOREPLIFETIME(ThisClass, CurrentBeerZ);
}

void ABeerFloodGimmick::Activate()
{
	const bool bWasActive = IsActive();

	Super::Activate();

	if (!bWasActive && HasAuthority())
	{
		CurrentBeerZ = GetBaseBeerZ();
		GetWorldTimerManager().SetTimer(CycleTimerHandle, this, &ThisClass::BeginWarning, FirstWarningDelay, false);

		UE_LOG(LogBeerFlood, Log, TEXT("Flood started. First warning in %.1fs, then every %.1fs"), FirstWarningDelay, RepeatInterval);
	}
}

void ABeerFloodGimmick::Deactivate()
{
	if (HasAuthority())
	{
		ReleaseAllDrowning();
		SetActorTickEnabled(false);
		CurrentBeerZ = GetBaseBeerZ();
		SetBeerFloodState(EBeerFloodState::Idle);
	}

	// Super clears the timers
	Super::Deactivate();
}

void ABeerFloodGimmick::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		ReleaseAllDrowning();
	}

	Super::EndPlay(EndPlayReason);
}

float ABeerFloodGimmick::GetBaseBeerZ() const
{
	return GetActorLocation().Z;
}

float ABeerFloodGimmick::GetPeakBeerZ() const
{
	return GetBaseBeerZ() + FloodHeight;
}

void ABeerFloodGimmick::SetBeerFloodState(const EBeerFloodState NewState)
{
	if (!HasAuthority() || BeerFloodState == NewState) return;

	BeerFloodState = NewState;

	UE_LOG(LogBeerFlood, Log, TEXT("Flood state %s"), *EnumHelper::EnumToString(NewState));

	OnRep_BeerFloodState();
	ForceNetUpdate();
}

void ABeerFloodGimmick::OnRep_BeerFloodState()
{
	// Tick only while the surface moves, or while debug draw needs it.
	bool bNeedsTick = BeerFloodState == EBeerFloodState::Rising || BeerFloodState == EBeerFloodState::Draining;
#if !UE_BUILD_SHIPPING
	bNeedsTick |= CVarBeerFloodDebug.GetValueOnGameThread() != 0;
#endif
	SetActorTickEnabled(bNeedsTick);

	OnBeerFloodStateChanged(BeerFloodState);
}

void ABeerFloodGimmick::BeginWarning()
{
	if (!HasAuthority()) return;

	SetBeerFloodState(EBeerFloodState::Warning);

	GetWorldTimerManager().SetTimer(CycleTimerHandle, this, &ThisClass::BeginWarning, RepeatInterval, false);
	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &ThisClass::BeginRising, WarningDuration, false);
}

void ABeerFloodGimmick::BeginRising()
{
	if (!HasAuthority()) return;

	PhaseElapsed = 0.f;
	SetBeerFloodState(EBeerFloodState::Rising);

	GetWorldTimerManager().SetTimer(DrowningTimerHandle, this, &ThisClass::UpdateDrowning, DrowningCheckInterval, true);
	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &ThisClass::BeginSustain, RisingDuration, false);
}

void ABeerFloodGimmick::BeginSustain()
{
	if (!HasAuthority()) return;

	CurrentBeerZ = GetPeakBeerZ();
	SetBeerFloodState(EBeerFloodState::Sustain);

	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &ThisClass::BeginDraining, SustainDuration, false);
}

void ABeerFloodGimmick::BeginDraining()
{
	if (!HasAuthority()) return;

	PhaseElapsed = 0.f;
	SetBeerFloodState(EBeerFloodState::Draining);

	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &ThisClass::EndBeerFlood, DrainingDuration, false);
}

void ABeerFloodGimmick::EndBeerFlood()
{
	if (!HasAuthority()) return;

	GetWorldTimerManager().ClearTimer(DrowningTimerHandle);

	CurrentBeerZ = GetBaseBeerZ();
	ReleaseAllDrowning();
	SetBeerFloodState(EBeerFloodState::Idle);
}

void ABeerFloodGimmick::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

#if !UE_BUILD_SHIPPING
	if (CVarBeerFloodDebug.GetValueOnGameThread() != 0)
	{
		DebugDrawGimmickState();
	}
#endif

	if (!HasAuthority()) return;

	if (BeerFloodState == EBeerFloodState::Rising)
	{
		PhaseElapsed += DeltaSeconds;
		const float Alpha = FMath::Clamp(PhaseElapsed / FMath::Max(0.05f, RisingDuration), 0.f, 1.f);
		CurrentBeerZ = FMath::Lerp(GetBaseBeerZ(), GetPeakBeerZ(), Alpha);
	}
	else if (BeerFloodState == EBeerFloodState::Draining)
	{
		PhaseElapsed += DeltaSeconds;
		const float Alpha = FMath::Clamp(PhaseElapsed / FMath::Max(0.05f, DrainingDuration), 0.f, 1.f);
		CurrentBeerZ = FMath::Lerp(GetPeakBeerZ(), GetBaseBeerZ(), Alpha);
	}
}

void ABeerFloodGimmick::UpdateDrowning()
{
	if (!HasAuthority()) return;

	// 상승/배수 중에는 수면이 움직이므로 이미 빠진 캐릭터의 기준 높이도 따라가야 한다
	for (const TWeakObjectPtr<ATromboneCharacterBase>& WeakCharacter : DrowningCharacters)
	{
		if (const ATromboneCharacterBase* Character = WeakCharacter.Get())
		{
			if (UTromboneRagdollComponent* Ragdoll = Character->GetRagdollComponent())
			{
				Ragdoll->SetWaterLevelZ(CurrentBeerZ);
			}
		}
	}

	for (TActorIterator<ATromboneCharacterBase> It(GetWorld()); It; ++It)
	{
		ATromboneCharacterBase* Character = *It;
		if (!Character) continue;

		if (DrowningCharacters.Contains(Character)) continue;

		const UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
		if (!Capsule) continue;

		const float FeetZ = Character->GetActorLocation().Z - Capsule->GetScaledCapsuleHalfHeight();
		if (FeetZ > CurrentBeerZ) continue;

		UTromboneRagdollComponent* Ragdoll = Character->GetRagdollComponent();
		if (!Ragdoll) continue;

		Ragdoll->SetAutoGetUpEnabled(false);
		if (!Ragdoll->IsRagdoll())
		{
			Ragdoll->StartRagdoll();
		}
		Ragdoll->SetFloatingEnabled(true, CurrentBeerZ);

		DrowningCharacters.Add(Character);
		Character->HandleDrowningStarted();

		UE_LOG(LogBeerFlood, Log, TEXT("%s started drowning (feet %.0f < beer %.0f)"), *Character->GetName(), FeetZ, CurrentBeerZ);
	}
}

void ABeerFloodGimmick::ReleaseAllDrowning()
{
	for (const TWeakObjectPtr<ATromboneCharacterBase>& WeakCharacter : DrowningCharacters)
	{
		ATromboneCharacterBase* Character = WeakCharacter.Get();
		if (!Character) continue;

		if (UTromboneRagdollComponent* Ragdoll = Character->GetRagdollComponent())
		{
			Ragdoll->SetFloatingEnabled(false);
			Ragdoll->SetAutoGetUpEnabled(true);
		}

		Character->HandleDrowningEnded();
	}

	DrowningCharacters.Reset();
}

void ABeerFloodGimmick::DebugDrawGimmickState() const
{
#if !UE_BUILD_SHIPPING
	if (!GEngine) return;

	const FString StateName = EnumHelper::EnumToString(BeerFloodState);

	TStringBuilder<512> Text;
	Text.Appendf(TEXT("── 침수 기믹 [%s] ──\n"), HasAuthority() ? TEXT("서버") : TEXT("클라"));
	Text.Appendf(TEXT("단계   %s   %s\n"), *StateName, IsActive() ? TEXT("") : TEXT("(비활성)"));
	Text.Appendf(TEXT("수위   %.0f   (최저 %.0f / 최고 %.0f)\n"), CurrentBeerZ, GetBaseBeerZ(), GetPeakBeerZ());

	if (HasAuthority())
	{
		Text.Appendf(TEXT("다음 전조까지 %.1fs\n"), FMath::Max(0.f, GetWorldTimerManager().GetTimerRemaining(CycleTimerHandle)));
		Text.Appendf(TEXT("빠진 인원   %d명\n"), DrowningCharacters.Num());
	}

	GEngine->AddOnScreenDebugMessage(static_cast<uint64>(GetUniqueID()), 1.f, FColor::Cyan, Text.ToString());

#endif
}
