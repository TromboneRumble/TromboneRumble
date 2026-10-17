// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Actors/Gimmick/Gravity/GravityGimmick.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AkGameplayStatics.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Components/MeshComponent.h"
#include "Data/Gimmick/GravityGimmickConfig.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "GameplayEffect.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "TromboneGamePlayTags.h"

#if !UE_BUILD_SHIPPING
TAutoConsoleVariable<int32> CVarGravityDebug(
	TEXT("Trombone.Gravity.Debug"),
	0,
	TEXT("1이면 중력 기믹의 상태와 남은 시간을 화면에 표시"));
#endif

namespace
{
	constexpr float PlayerEffectRefreshInterval = 0.5f;
}

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

#if !UE_BUILD_SHIPPING
	IConsoleVariable* DebugVariable = CVarGravityDebug.AsVariable();
	DebugVariable->OnChangedDelegate().AddUObject(this, &ThisClass::HandleDebugCVarChanged);
	HandleDebugCVarChanged(DebugVariable);
#endif
}

void AGravityGimmick::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// The parameter is global and outlives the level, so the next map would start muffled
	SetMuffle(0.f, 0.f);

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
	SetActorTickEnabled(bDebugDraw);
}

void AGravityGimmick::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

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
	// The gravity has no timer. A sequence of the stage data calls ForceTrigger when it is its turn
	Super::Activate();
}

void AGravityGimmick::Deactivate()
{
	if (HasAuthority())
	{
		const bool bWasRunning = State != EGravityState::Idle;

		RemoveAllEffects();
		SetState(EGravityState::Idle);

		// An event stopped half way still ends its turn, so a sequence waiting for it does not stall
		if (bWasRunning)
		{
			NotifyEventFinished();
		}
	}

	// Super clears the timers
	Super::Deactivate();
}

void AGravityGimmick::ForceTrigger()
{
	if (!HasAuthority() || State != EGravityState::Idle) return;

	StartWarning();
}

void AGravityGimmick::StartWarning()
{
	if (!HasAuthority()) return;

	// No warning time means no warning. A zero timer would never fire, and the gravity would never change
	const float WarningDuration = GetConfig<UGravityGimmickConfig>().WarningDuration;
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

	StartPhaseTimer(GetWorldTimerManager(), PhaseTimerHandle, FTimerDelegate::CreateUObject(this, &ThisClass::EndActive), GetConfig<UGravityGimmickConfig>().ActiveDuration);
}

void AGravityGimmick::EndActive()
{
	if (!HasAuthority()) return;

	RemoveAllEffects();
	SetState(EGravityState::Idle);

	NotifyEventFinished();
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

	UpdateAudio();
	UpdatePlayerEffects();

	OnGravityStateChanged(State, GravityMultiplier);
}

void AGravityGimmick::UpdateAudio()
{
	const UGravityGimmickConfig& Config = GetConfig<UGravityGimmickConfig>();
	switch (State)
	{
	case EGravityState::Warning:
		// The air escapes over the whole warning, so the mix is fully muffled by the time the gravity changes.
		// The sound fills the whole station, so it plays without a position and every player hears it the same
		if (WarningSound)
		{
			UAkGameplayStatics::PostEvent(WarningSound, nullptr, 0, FOnAkPostEventCallback());
		}
		SetMuffle(100.f, Config.WarningDuration);
		break;

	case EGravityState::Active:
		// Already there after a warning. Needed when there was no warning, or for a client that joined after it
		SetMuffle(100.f, 0.f);
		break;

	case EGravityState::Idle:
		SetMuffle(0.f, Config.MuffleOutDuration);
		break;
	}
}

void AGravityGimmick::SetMuffle(const float Value, const float Duration) const
{
	if (MuffleParameterName.IsNone()) return;

	// Wwise moves the value in a straight line over the given time, so nothing needs to run every frame
	const int32 DurationMs = FMath::RoundToInt(FMath::Max(Duration, 0.f) * 1000.f);
	UAkGameplayStatics::SetRTPCValue(nullptr, Value, DurationMs, nullptr, MuffleParameterName);
}

void AGravityGimmick::UpdatePlayerEffects()
{
	if (State == EGravityState::Active && (OutlineMaterial || PlayerVFX))
	{
		RefreshPlayerEffects();
		GetWorldTimerManager().SetTimer(PlayerEffectRefreshTimerHandle, this, &ThisClass::RefreshPlayerEffects, PlayerEffectRefreshInterval, true);
	}
	else
	{
		GetWorldTimerManager().ClearTimer(PlayerEffectRefreshTimerHandle);
		ClearPlayerEffects();
	}
}

void AGravityGimmick::RefreshPlayerEffects()
{
	const AGameStateBase* GameState = GetWorld()->GetGameState();
	if (!GameState) return;

	TSet<TWeakObjectPtr<UMeshComponent>> Current;
	for (const APlayerState* PlayerState : GameState->PlayerArray)
	{
		APawn* Pawn = PlayerState ? PlayerState->GetPawn() : nullptr;
		if (!Pawn) continue;

		const ACharacter* Character = Cast<ACharacter>(Pawn);
		if (PlayerVFX && Character && !PlayerVFXComponents.Contains(Pawn))
		{
			// Attached to the body mesh, so the emitter samples the body and not a customization part
			UNiagaraComponent* VFX = UNiagaraFunctionLibrary::SpawnSystemAttached(PlayerVFX, Character->GetMesh(), NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, false);
			PlayerVFXComponents.Add(Pawn, VFX);
		}

		if (!OutlineMaterial) continue;

		// Customization parts are components of the pawn, and the held instrument and weapon are attached actors
		TArray<AActor*> Actors = { Pawn };
		Pawn->GetAttachedActors(Actors, false, true);

		for (const AActor* Actor : Actors)
		{
			TInlineComponentArray<UMeshComponent*> Meshes(Actor);
			for (UMeshComponent* Mesh : Meshes)
			{
				// Another effect uses this overlay, so we leave it alone
				UMaterialInterface* Overlay = Mesh->GetOverlayMaterial();
				if (Overlay && Overlay != OutlineMaterial) continue;

				Mesh->SetOverlayMaterial(OutlineMaterial);
				Current.Add(Mesh);
			}
		}
	}

	// A dropped instrument loses the outline
	for (const TWeakObjectPtr<UMeshComponent>& Mesh : OutlinedMeshes)
	{
		if (Mesh.IsValid() && !Current.Contains(Mesh) && Mesh->GetOverlayMaterial() == OutlineMaterial)
		{
			Mesh->SetOverlayMaterial(nullptr);
		}
	}
	OutlinedMeshes = MoveTemp(Current);
}

void AGravityGimmick::ClearPlayerEffects()
{
	for (const TWeakObjectPtr<UMeshComponent>& Mesh : OutlinedMeshes)
	{
		if (Mesh.IsValid() && Mesh->GetOverlayMaterial() == OutlineMaterial)
		{
			Mesh->SetOverlayMaterial(nullptr);
		}
	}
	OutlinedMeshes.Empty();

	for (const TPair<TWeakObjectPtr<APawn>, TWeakObjectPtr<UNiagaraComponent>>& Pair : PlayerVFXComponents)
	{
		// Removed at once, so it disappears together with the outline
		if (UNiagaraComponent* VFX = Pair.Value.Get())
		{
			VFX->DestroyComponent();
		}
	}
	PlayerVFXComponents.Empty();
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

	// Idle waits for its turn in the sequence, so only the phases have a time left
	const float Seconds = GetWorldTimerManager().GetTimerRemaining(PhaseTimerHandle);
	const FString Remaining = State == EGravityState::Idle ? FString(TEXT("순서 대기")) : Seconds >= 0.f ? FString::Printf(TEXT("%.1fs"), Seconds) : FString(TEXT("타이머 없음"));

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
