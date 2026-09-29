// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Actors/Gimmick/Gravity/GravityGimmick.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Components/LightComponent.h"
#include "Data/Gimmick/GravityGimmickConfig.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "GameplayEffect.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Net/UnrealNetwork.h"
#include "TromboneGamePlayTags.h"
#include "Utilities/TromboneLogs.h"

#if !UE_BUILD_SHIPPING
TAutoConsoleVariable<int32> CVarGravityDebug(
	TEXT("Trombone.Gravity.Debug"),
	0,
	TEXT("1이면 중력 기믹의 상태와 남은 시간을 화면에 표시"));
#endif

AGravityGimmick::AGravityGimmick()
{
	bReplicates = true;
	GimmickType = EGimmickType::Gravity;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AGravityGimmick::BeginPlay()
{
	Super::BeginPlay();

	// The state can arrive before BeginPlay on a client that joins during a warning, so the lights follow it right away
	CollectWarningLights();
	RefreshWarningLights();

#if !UE_BUILD_SHIPPING
	IConsoleVariable* DebugVariable = CVarGravityDebug.AsVariable();
	DebugVariable->OnChangedDelegate().AddUObject(this, &ThisClass::HandleDebugCVarChanged);
	HandleDebugCVarChanged(DebugVariable);
#endif
}

void AGravityGimmick::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
#if !UE_BUILD_SHIPPING
	CVarGravityDebug.AsVariable()->OnChangedDelegate().RemoveAll(this);
#endif

	Super::EndPlay(EndPlayReason);
}

void AGravityGimmick::HandleDebugCVarChanged(IConsoleVariable* Variable)
{
	bDebugDraw = Variable && Variable->GetInt() != 0;
	UpdateTickEnabled();
}

void AGravityGimmick::UpdateTickEnabled()
{
	SetActorTickEnabled(bDebugDraw || (bWarningLightsOn && !WarningLights.IsEmpty()));
}

void AGravityGimmick::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (State == EGravityState::Warning)
	{
		UpdateWarningLightFlicker();
	}
	else if (FadeOutStartTime >= 0.f)
	{
		UpdateWarningLightFadeOut();
	}

	if (bDebugDraw)
	{
		DebugDraw();
	}
}

void AGravityGimmick::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, State);
}

void AGravityGimmick::Activate()
{
	const bool bWasActive = IsActive();

	Super::Activate();

	if (!bWasActive && HasAuthority())
	{
		ScheduleNext(GetConfig<UGravityGimmickConfig>().Schedule.PickFirstDelay());
	}
}

void AGravityGimmick::Deactivate()
{
	if (HasAuthority())
	{
		RemoveAllEffects();
		SetState(EGravityState::Idle);
	}

	// Super clears the timers
	Super::Deactivate();
}

void AGravityGimmick::ForceTrigger()
{
	if (!HasAuthority() || State != EGravityState::Idle) return;

	GetWorldTimerManager().ClearTimer(ScheduleTimerHandle);
	StartWarning();
}

void AGravityGimmick::ScheduleNext(const float Delay)
{
	if (!HasAuthority()) return;

	GetWorldTimerManager().ClearTimer(ScheduleTimerHandle);
	GetWorldTimerManager().SetTimer(ScheduleTimerHandle, this, &ThisClass::StartWarning, Delay, false);
}

void AGravityGimmick::StartWarning()
{
	if (!HasAuthority()) return;

	// No warning time means no warning. A zero timer would never fire, and the gravity would never change
	const float WarningDuration = GetConfig<UGravityGimmickConfig>().Schedule.WarningDuration;
	if (WarningDuration <= 0.f)
	{
		StartActive();
		return;
	}

	SetState(EGravityState::Warning);

	GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &ThisClass::StartActive, WarningDuration, false);
}

void AGravityGimmick::StartActive()
{
	if (!HasAuthority()) return;

	ApplyEffectToAllPlayers();
	SetState(EGravityState::Active);

	GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &ThisClass::EndActive, GetConfig<UGravityGimmickConfig>().ActiveDuration, false);
}

void AGravityGimmick::EndActive()
{
	if (!HasAuthority()) return;

	RemoveAllEffects();
	SetState(EGravityState::Idle);

	ScheduleNext(GetConfig<UGravityGimmickConfig>().Schedule.PickInterval());
}

void AGravityGimmick::SetState(const EGravityState NewState)
{
	if (!HasAuthority() || State == NewState) return;

	State = NewState;

	OnRep_State();
	ForceNetUpdate();
}

void AGravityGimmick::OnRep_State()
{
	HandleStateChanged();
}

void AGravityGimmick::HandleStateChanged()
{
	if (GetNetMode() == NM_DedicatedServer || !GetWorld() || GetWorld()->bIsTearingDown) return;

	const float GravityMultiplier = GetConfig<UGravityGimmickConfig>().GravityMultiplier;

	if (GimmickParameterCollection)
	{
		if (!StateParameterName.IsNone())
		{
			UKismetMaterialLibrary::SetScalarParameterValue(this, GimmickParameterCollection, StateParameterName, static_cast<float>(State));
		}
		if (!MultiplierParameterName.IsNone())
		{
			UKismetMaterialLibrary::SetScalarParameterValue(this, GimmickParameterCollection, MultiplierParameterName, GravityMultiplier);
		}
	}

	RefreshWarningLights();

	OnGravityStateChanged(State, GravityMultiplier);
}

void AGravityGimmick::CollectWarningLights()
{
	// Lights only change what players see, so a dedicated server skips them
	if (GetNetMode() == NM_DedicatedServer || WarningLightTag.IsNone()) return;

	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (!It->ActorHasTag(WarningLightTag)) continue;

		TInlineComponentArray<ULightComponent*> Lights(*It);
		for (ULightComponent* Light : Lights)
		{
			if (Light->Mobility == EComponentMobility::Static)
			{
				UE_LOG(LogGimmick, Warning, TEXT("[AGravityGimmick] %s is a static light and cannot flicker. Set its Mobility to Movable."), *It->GetName());
				continue;
			}

			WarningLights.Add({ Light, Light->Intensity });
		}
	}

	if (WarningLights.IsEmpty())
	{
		UE_LOG(LogGimmick, Log, TEXT("[AGravityGimmick] No light has the tag %s, so the warning shows no lights."), *WarningLightTag.ToString());
	}
}

void AGravityGimmick::RefreshWarningLights()
{
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	if (State == EGravityState::Warning)
	{
		WarningStartTime = Now;
		FadeOutStartTime = -1.f;
		bWarningLightsOn = true;

		for (const FWarningLight& Entry : WarningLights)
		{
			if (ULightComponent* Light = Entry.Light.Get())
			{
				Light->SetVisibility(true);
			}
		}
		SetWarningLightRatio(0.f);
	}
	else if (!bWarningLightsOn)
	{
		// Nothing to fade, as when the gimmick starts in Idle
		TurnOffWarningLights();
	}
	else if (FadeOutStartTime < 0.f)
	{
		// Active and then Idle both come after the warning, so only the first one starts the fade
		FadeOutStartTime = Now;
		FadeOutStartRatio = CurrentWarningLightRatio;
	}

	UpdateTickEnabled();
}

void AGravityGimmick::UpdateWarningLightFlicker()
{
	const UGravityGimmickConfig& Config = GetConfig<UGravityGimmickConfig>();
	const float Elapsed = GetWorld()->GetTimeSeconds() - WarningStartTime;

	const float Frequency = FMath::Max(Config.WarningLightFrequency, KINDA_SMALL_NUMBER);

	// The wave starts dark, so the lights turn on smoothly
	const float Wave = 0.5f - 0.5f * FMath::Cos(2.f * PI * Frequency * Elapsed);

	// The first rise goes from 0 and every later swing from the minimum. Both meet at full brightness, so there is no jump
	const float HalfPeriod = 0.5f / Frequency;
	const float MinRatio = Elapsed < HalfPeriod ? 0.f : Config.WarningLightMinRatio;
	SetWarningLightRatio(FMath::Lerp(MinRatio, 1.f, Wave));
}

void AGravityGimmick::UpdateWarningLightFadeOut()
{
	// The fade takes half a flicker, the time the flicker takes to go from bright to dark, so it moves at the same pace
	const float Duration = 0.5f / FMath::Max(GetConfig<UGravityGimmickConfig>().WarningLightFrequency, KINDA_SMALL_NUMBER);
	const float Alpha = (GetWorld()->GetTimeSeconds() - FadeOutStartTime) / Duration;

	if (Alpha >= 1.f)
	{
		TurnOffWarningLights();
		UpdateTickEnabled();
		return;
	}

	// The same sine shape as the flicker, so the last dimming looks like one more swing of it
	SetWarningLightRatio(FMath::InterpSinInOut(FadeOutStartRatio, 0.f, Alpha));
}

void AGravityGimmick::SetWarningLightRatio(const float Ratio)
{
	CurrentWarningLightRatio = Ratio;

	for (const FWarningLight& Entry : WarningLights)
	{
		if (ULightComponent* Light = Entry.Light.Get())
		{
			Light->SetIntensity(Entry.BaseIntensity * Ratio);
		}
	}
}

void AGravityGimmick::TurnOffWarningLights()
{
	bWarningLightsOn = false;
	FadeOutStartTime = -1.f;

	// The intensity goes back to the placed value, so the next warning starts from it
	for (const FWarningLight& Entry : WarningLights)
	{
		if (ULightComponent* Light = Entry.Light.Get())
		{
			Light->SetVisibility(false);
			Light->SetIntensity(Entry.BaseIntensity);
		}
	}
	CurrentWarningLightRatio = 1.f;
}

void AGravityGimmick::ApplyEffectToAllPlayers()
{
	if (!HasAuthority() || !GravityEffectClass) return;

	const AGameStateBase* GameState = GetWorld() ? GetWorld()->GetGameState() : nullptr;
	if (!GameState) return;

	for (const APlayerState* PlayerState : GameState->PlayerArray)
	{
		ADefaultTromboneCharacter* Character = PlayerState ? Cast<ADefaultTromboneCharacter>(PlayerState->GetPawn()) : nullptr;
		UAbilitySystemComponent* ASC = Character ? Character->GetAbilitySystemComponent() : nullptr;
		if (!ASC || ActiveEffects.Contains(Character)) continue;

		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		Context.AddSourceObject(this);

		const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(GravityEffectClass, 1.f, Context);
		if (!SpecHandle.IsValid()) continue;
		SpecHandle.Data->SetSetByCallerMagnitude(TromboneGamePlayTags::Trombone_Gimmick_Gravity_Scale, GetConfig<UGravityGimmickConfig>().GravityMultiplier);

		const FActiveGameplayEffectHandle Handle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
		if (Handle.IsValid())
		{
			ActiveEffects.Add(Character, Handle);
		}
	}
}

void AGravityGimmick::RemoveAllEffects()
{
	if (!HasAuthority()) return;

	for (const auto& Pair : ActiveEffects)
	{
		const IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Pair.Key.Get());
		UAbilitySystemComponent* ASC = ASCInterface ? ASCInterface->GetAbilitySystemComponent() : nullptr;
		if (ASC && Pair.Value.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(Pair.Value);
		}
	}
	ActiveEffects.Empty();
}

void AGravityGimmick::DebugDraw() const
{
#if !UE_BUILD_SHIPPING
	if (!GEngine || !HasAuthority()) return;

	const FTimerHandle& Handle = (State == EGravityState::Idle) ? ScheduleTimerHandle : PhaseTimerHandle;
	const float Seconds = GetWorldTimerManager().GetTimerRemaining(Handle);
	const FString Remaining = Seconds >= 0.f ? FString::Printf(TEXT("%.1fs"), Seconds) : TEXT("타이머 없음");

	FString Gravities;
	if (const AGameStateBase* GameState = GetWorld() ? GetWorld()->GetGameState() : nullptr)
	{
		for (const APlayerState* PlayerState : GameState->PlayerArray)
		{
			const ACharacter* Character = PlayerState ? Cast<ACharacter>(PlayerState->GetPawn()) : nullptr;
			if (!Character || !Character->GetCharacterMovement()) continue;
			Gravities += FString::Printf(TEXT("%s%.2f"), Gravities.IsEmpty() ? TEXT("") : TEXT(" / "), Character->GetCharacterMovement()->GravityScale);
		}
	}

	const FString Text = FString::Printf(TEXT("[중력 기믹] 상태 %s | 남은 시간 %s | 설정 배율 x%.2f | 적용 인원 %d | 캐릭터 중력 %s"),
		*UEnum::GetDisplayValueAsText(State).ToString(),
		*Remaining,
		GetConfig<UGravityGimmickConfig>().GravityMultiplier,
		ActiveEffects.Num(),
		Gravities.IsEmpty() ? TEXT("-") : *Gravities);
	GEngine->AddOnScreenDebugMessage(static_cast<uint64>(GetUniqueID()), 1.f, FColor::Cyan, Text);
#endif
}
