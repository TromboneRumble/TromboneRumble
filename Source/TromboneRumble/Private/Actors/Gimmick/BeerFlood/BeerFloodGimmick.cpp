// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Actors/Gimmick/BeerFlood/BeerFloodGimmick.h"
#include "AkAudioEvent.h"
#include "AkComponent.h"
#include "AkGameplayStatics.h"
#include "Characters/TromboneCharacterBase.h"
#include "Components/ActorComponents/GuideSignalComponent.h"
#include "Components/ActorComponents/TromboneRagdollComponent.h"
#include "Components/CapsuleComponent.h"
#include "Data/Gimmick/BeerFloodGimmickConfig.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"
#include "Subsystems/WorldSubsystem/FloatableSubsystem.h"
#include "Utilities/EnumHelper.h"
#include "Utilities/TromboneLogs.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"

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

	GuideSignal = CreateDefaultSubobject<UGuideSignalComponent>(TEXT("GuideSignal"));
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

		const float FirstDelay = GetConfig<UBeerFloodGimmickConfig>().FirstWarningDelay;
		GetWorldTimerManager().SetTimer(CycleTimerHandle, this, &ThisClass::BeginWarning, FirstDelay, false);

		UE_LOG(LogBeerFlood, Log, TEXT("Flood started. First warning in %.1fs"), FirstDelay);
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
	EndFlood();

	// The pour component lives in the world, not on this actor, so it has to go by hand
	if (PourAkComponent)
	{
		StopPourSound();
		PourAkComponent->DestroyComponent();
		PourAkComponent = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

float ABeerFloodGimmick::GetBaseBeerZ() const
{
	return GetActorLocation().Z;
}

float ABeerFloodGimmick::GetPeakBeerZ() const
{
	return GetBaseBeerZ() + GetConfig<UBeerFloodGimmickConfig>().FloodHeight;
}

void ABeerFloodGimmick::SetBeerFloodState(const EBeerFloodState NewState)
{
	if (!HasAuthority() || BeerFloodState == NewState) return;

	BeerFloodState = NewState;
	GuideSignal->SetGuiding(NewState == EBeerFloodState::Warning);

	UE_LOG(LogBeerFlood, Log, TEXT("Flood state %s"), *EnumHelper::EnumToString(NewState));

	OnRep_BeerFloodState();
	ForceNetUpdate();
}

void ABeerFloodGimmick::OnRep_BeerFloodState()
{
	PhaseElapsed = 0.f;

	// Tick while any phase runs, for the surface and for the material progress, or while debug draw needs it.
	bool bNeedsTick = BeerFloodState != EBeerFloodState::Idle;
#if !UE_BUILD_SHIPPING
	bNeedsTick |= CVarBeerFloodDebug.GetValueOnGameThread() != 0;
#endif
	SetActorTickEnabled(bNeedsTick);

	WriteMaterialParameters();
	PlayPhaseSounds();

	// The tick is off in Sustain, so the final Z of the rise gets pushed here
	if (BeerFloodState != EBeerFloodState::Idle)
	{
		PushWaterLevel();
	}
	else
	{
		EndFlood();
	}

	OnBeerFloodStateChanged(BeerFloodState);
}

void ABeerFloodGimmick::BeginWarning()
{
	if (!HasAuthority()) return;

	SetBeerFloodState(EBeerFloodState::Warning);

	const UBeerFloodGimmickConfig& Config = GetConfig<UBeerFloodGimmickConfig>();

	// The period runs from warning to warning, so changing a phase time does not move the next flood
	// A period shorter than one flood would start a new warning in the middle of this flood
	// The small margin makes the flood end first when both times are equal
	const float Period = FMath::Max(Config.Period, Config.GetFloodDuration() + 0.1f);
	GetWorldTimerManager().SetTimer(CycleTimerHandle, this, &ThisClass::BeginWarning, Period, false);

	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &ThisClass::BeginRising, Config.WarningDuration, false);
}

void ABeerFloodGimmick::BeginRising()
{
	if (!HasAuthority()) return;

	SetBeerFloodState(EBeerFloodState::Rising);

	GetWorldTimerManager().SetTimer(DrowningTimerHandle, this, &ThisClass::UpdateDrowning, DrowningCheckInterval, true);
	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &ThisClass::BeginSustain, GetConfig<UBeerFloodGimmickConfig>().RisingDuration, false);
}

void ABeerFloodGimmick::BeginSustain()
{
	if (!HasAuthority()) return;

	CurrentBeerZ = GetPeakBeerZ();
	SetBeerFloodState(EBeerFloodState::Sustain);

	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &ThisClass::BeginDraining, GetConfig<UBeerFloodGimmickConfig>().SustainDuration, false);
}

void ABeerFloodGimmick::BeginDraining()
{
	if (!HasAuthority()) return;

	SetBeerFloodState(EBeerFloodState::Draining);

	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &ThisClass::EndBeerFlood, GetConfig<UBeerFloodGimmickConfig>().DrainingDuration, false);
}

void ABeerFloodGimmick::EndBeerFlood()
{
	if (!HasAuthority()) return;

	GetWorldTimerManager().ClearTimer(DrowningTimerHandle);

	CurrentBeerZ = GetBaseBeerZ();
	ReleaseAllDrowning();
	SetBeerFloodState(EBeerFloodState::Idle);
}

void ABeerFloodGimmick::ForceTrigger()
{
	if (!HasAuthority() || BeerFloodState != EBeerFloodState::Idle) return;

	GetWorldTimerManager().ClearTimer(CycleTimerHandle);
	BeginWarning();
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

	if (BeerFloodState != EBeerFloodState::Idle)
	{
		PhaseElapsed += DeltaSeconds;
	}

	if (HasAuthority())
	{
		if (BeerFloodState == EBeerFloodState::Rising)
		{
			CurrentBeerZ = FMath::Lerp(GetBaseBeerZ(), GetPeakBeerZ(), GetPhaseProgress());
		}
		else if (BeerFloodState == EBeerFloodState::Draining)
		{
			CurrentBeerZ = FMath::Lerp(GetPeakBeerZ(), GetBaseBeerZ(), GetPhaseProgress());
		}
	}

	if (BeerFloodState == EBeerFloodState::Rising || BeerFloodState == EBeerFloodState::Draining)
	{
		PushWaterLevel();
	}

	WriteMaterialParameters();
}

float ABeerFloodGimmick::GetPhaseDuration() const
{
	const UBeerFloodGimmickConfig& Config = GetConfig<UBeerFloodGimmickConfig>();
	switch (BeerFloodState)
	{
	case EBeerFloodState::Warning:  return Config.WarningDuration;
	case EBeerFloodState::Rising:   return Config.RisingDuration;
	case EBeerFloodState::Sustain:  return Config.SustainDuration;
	case EBeerFloodState::Draining: return Config.DrainingDuration;
	default:                        return 0.f;
	}
}

float ABeerFloodGimmick::GetPhaseProgress() const
{
	const float Duration = GetPhaseDuration();
	return Duration > 0.f ? FMath::Clamp(PhaseElapsed / Duration, 0.f, 1.f) : 0.f;
}

void ABeerFloodGimmick::WriteMaterialParameters() const
{
	if (!GimmickParameterCollection || GetNetMode() == NM_DedicatedServer || !GetWorld() || GetWorld()->bIsTearingDown) return;

	if (!StateParameterName.IsNone())
	{
		UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), GimmickParameterCollection, StateParameterName, static_cast<float>(BeerFloodState));
	}
	if (!ProgressParameterName.IsNone())
	{
		UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), GimmickParameterCollection, ProgressParameterName, GetPhaseProgress());
	}
}

void ABeerFloodGimmick::PlayPhaseSounds()
{
	if (GetNetMode() == NM_DedicatedServer) return;

	// The rise and drain sounds cover the whole map, so they play without a position like the menu music
	switch (BeerFloodState)
	{
	case EBeerFloodState::Rising:
		StartPourSound();
		if (WaterRiseEvent)
		{
			UAkGameplayStatics::PostEvent(WaterRiseEvent, nullptr, 0, FOnAkPostEventCallback());
		}
		break;

	case EBeerFloodState::Draining:
		if (WaterDrainEvent)
		{
			UAkGameplayStatics::PostEvent(WaterDrainEvent, nullptr, 0, FOnAkPostEventCallback());
		}
		// The pour already stopped in Sustain, but a forced stop skips that phase
		[[fallthrough]];
	case EBeerFloodState::Sustain:
	case EBeerFloodState::Idle:
		StopPourSound();
		break;

	default:
		break;
	}
}

void ABeerFloodGimmick::StartPourSound()
{
	if (!PourStartEvent) return;

	if (!PourAkComponent)
	{
		const FVector Location = GetActorTransform().TransformPosition(PourSoundOffset);
		PourAkComponent = UAkGameplayStatics::SpawnAkComponentAtLocation(this, nullptr, Location, FRotator::ZeroRotator, false, FString(), false);
	}
	if (PourAkComponent)
	{
		PourAkComponent->PostAkEvent(PourStartEvent, 0, FOnAkPostEventCallback());
	}
}

void ABeerFloodGimmick::StopPourSound()
{
	if (PourAkComponent && PourStopEvent)
	{
		PourAkComponent->PostAkEvent(PourStopEvent, 0, FOnAkPostEventCallback());
	}
}

void ABeerFloodGimmick::PushWaterLevel()
{
	if (UFloatableSubsystem* Floatables = GetWorld() ? GetWorld()->GetSubsystem<UFloatableSubsystem>() : nullptr)
	{
		Floatables->SetWaterLevel(CurrentBeerZ);
	}
}

void ABeerFloodGimmick::EndFlood()
{
	if (UFloatableSubsystem* Floatables = GetWorld() ? GetWorld()->GetSubsystem<UFloatableSubsystem>() : nullptr)
	{
		Floatables->EndFlood();
	}
}

void ABeerFloodGimmick::UpdateDrowning()
{
	if (!HasAuthority()) return;

	for (TActorIterator<ATromboneCharacterBase> It(GetWorld()); It; ++It)
	{
		ATromboneCharacterBase* Character = *It;
		if (!Character) continue;

		if (DrowningCharacters.Contains(Character)) continue;

		UTromboneRagdollComponent* Ragdoll = Character->GetRagdollComponent();
		if (!Ragdoll) continue;

		// The capsule stays where the ragdoll began, so read the pelvis instead.
		float BodyZ;
		if (Ragdoll->IsRagdoll())
		{
			BodyZ = Character->GetPelvisLocation().Z;
		}
		else
		{
			const UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
			if (!Capsule) continue;

			BodyZ = Character->GetActorLocation().Z - Capsule->GetScaledCapsuleHalfHeight();
		}
		if (BodyZ > CurrentBeerZ) continue;

		Ragdoll->SetAutoGetUpEnabled(false);
		if (!Ragdoll->IsRagdoll())
		{
			Ragdoll->StartRagdoll();
		}

		Character->AddBlock(ECharacterBlockReason::Ragdoll);

		DrowningCharacters.Add(Character);
		Character->HandleDrowningStarted();

		UE_LOG(LogBeerFlood, Log, TEXT("%s started drowning (body %.0f < beer %.0f)"), *Character->GetName(), BodyZ, CurrentBeerZ);
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

	if (const UFloatableSubsystem* Floatables = GetWorld() ? GetWorld()->GetSubsystem<UFloatableSubsystem>() : nullptr)
	{
		Text.Appendf(TEXT("뜨는 물체   %d개 (젖음 %d)\n"), Floatables->GetCount(), Floatables->GetWetCount());
	}

	GEngine->AddOnScreenDebugMessage(static_cast<uint64>(GetUniqueID()), 1.f, FColor::Cyan, Text.ToString());

#endif
}
