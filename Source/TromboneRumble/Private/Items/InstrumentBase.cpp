// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/InstrumentBase.h"
#include "Subsystems/RhythmSubsystem.h"
#include "Framework/DefaultPlayerState.h"
#include "Data/InstrumentScoreData.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AkGameplayStatics.h"
#include "Data/RhythmScoreAttributeSet.h"
#include "Actors/InstrumentIndicator.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Components/WidgetComponent.h"
#include "Framework/InGameState.h"
#include "UI/UserWidgets/OnScreenIndicator/OSI_WidgetBase.h"
#include "UI/UserWidgets/Rhythm/ComboWidget/RhythmComboWidgetBase.h"
#include "Utilities/DebugHelper.h"

AInstrumentBase::AInstrumentBase()
{
}

void AInstrumentBase::BeginPlay()
{
	Super::BeginPlay();
	if (IndicatorClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		IndicatorInstance = GetWorld()->SpawnActor<AInstrumentIndicator>(IndicatorClass, GetActorLocation() + IndicatorOffset, FRotator::ZeroRotator, SpawnParams);
		if (IndicatorInstance)
		{
			IndicatorInstance->InitInstrument(this, IndicatorOffset);
		}
	}

	if (IndicatorWidgetClass)
	{
		TryCreateIndicatorWidget();
	}

	if (AInGameState* InGameState = GetWorld()->GetGameState<AInGameState>())
	{
		InGameState->OnInGameStateChanged.AddDynamic(this, &ThisClass::HandleInGameStateChanged);
	}
}

void AInstrumentBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (IsValid(IndicatorInstance))
	{
		IndicatorInstance->Destroy();
		IndicatorInstance = nullptr;
	}
	if (IndicatorWidgetInstance.Get())
	{
		IndicatorWidgetInstance->RemoveFromParent();
		IndicatorWidgetInstance = nullptr;
	}
}

void AInstrumentBase::OnRep_Equipped()
{
	Super::OnRep_Equipped();
	if (bIsEquipped)
	{
		if (IsOwnerLocallyControlled())
		{
			BindToRhythmSubsystem(true);
			if (ADefaultTromboneCharacter* TromboneCharacter = Cast<ADefaultTromboneCharacter>(CurrentOwner))
			{
				UWidgetComponent* WidgetComponent = TromboneCharacter->GetComboWidgetComponent();
				if (ComboWidgetClass && WidgetComponent->GetWidgetClass() != ComboWidgetClass)
				{
					WidgetComponent->SetWidgetClass(ComboWidgetClass);
					WidgetComponent->InitWidget();
					UUserWidget* NewWidget = WidgetComponent->GetUserWidgetObject();
					if (URhythmComboWidgetBase* RhythmComboWidgetBase = Cast<URhythmComboWidgetBase>(NewWidget))
					{
						ComboWidgetInstance = RhythmComboWidgetBase;
						RhythmComboWidgetBase->Init(this);
					}
				}
			}
		}
	}
	else
	{
		RemoveBuff();
		BindToRhythmSubsystem(false);
		if (IsOwnerLocallyControlled())
		{
			if (ADefaultTromboneCharacter* TromboneCharacter = Cast<ADefaultTromboneCharacter>(CurrentOwner))
			{
				UWidgetComponent* WidgetComponent = TromboneCharacter->GetComboWidgetComponent();
				if (WidgetComponent->GetWidgetClass())
				{
					WidgetComponent->SetWidgetClass(nullptr);
				}
			}
		}
		if (InstrumentDropSound)
		{
			UAkGameplayStatics::PostEvent(InstrumentDropSound, this, 0, FOnAkPostEventCallback());
		}
	}
	TryUpdateIndicatorVisibility();
}

void AInstrumentBase::TryUpdateIndicatorVisibility()
{
	GetWorld()->GetTimerManager().ClearTimer(IndicatorRetryTimerHandle);
	bool bIsReady = IsValid(IndicatorInstance) && IndicatorWidgetInstance.Get();
	if (!bIsReady)
	{
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(
				IndicatorRetryTimerHandle,
				this,
				&AInstrumentBase::TryUpdateIndicatorVisibility,
				0.05f,
				false
			);
		}
		return;
	}

	if (bIsEquipped)
	{
		IndicatorInstance->GetRootComponent()->SetVisibility(false, true);
		IndicatorWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		IndicatorInstance->GetRootComponent()->SetVisibility(true, true);
		IndicatorWidgetInstance->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void AInstrumentBase::HandleInGameStateChanged(EInGameState InGameState)
{
	if (InGameState == EInGameState::Play)
	{
		TryUpdateIndicatorVisibility();
	}
}

void AInstrumentBase::ApplyBuff(TSubclassOf<UGameplayEffect> BuffClass)
{
	if (!BuffClass || ActiveBuffHandle.IsValid() || !CurrentOwner) return;

	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(CurrentOwner))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			Context.AddSourceObject(this);
			ActiveBuffHandle = ASC->ApplyGameplayEffectToSelf(BuffClass->GetDefaultObject<UGameplayEffect>(), 1.f, Context);
			if (ActiveBuffHandle.IsValid() && IsOwnerLocallyControlled())
			{
				if (BuffActivationSound)
				{
					UE_LOG(LogTemp, Warning, TEXT("BuffActivatedSoundCalled"));
					UAkGameplayStatics::PostEvent(BuffActivationSound, this, 0, FOnAkPostEventCallback());
				}
				OnBuffStateChanged.Broadcast(true);
				FString DebugMsg = FString::Printf(TEXT(">>> [Buff ON] %s Applied!"), *BuffClass->GetName());
				Debug::Print(DebugMsg);
			}
		}
	}
}

void AInstrumentBase::RemoveBuff()
{
	if (ActiveBuffHandle.IsValid() && CurrentOwner)
	{
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(CurrentOwner))
		{
			if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
			{
				ASC->RemoveActiveGameplayEffect(ActiveBuffHandle);
			}
			if (IsOwnerLocallyControlled())
			{
				OnBuffStateChanged.Broadcast(false);
				Debug::Print(TEXT("<<< [Buff OFF] Buff Removed"));
			}
		}
	}
	ActiveBuffHandle.Invalidate();
}

float AInstrumentBase::GetGradeMultiplier() const
{
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(CurrentOwner))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			bool bFound;
			float Val = ASC->GetGameplayAttributeValue(URhythmScoreAttributeSet::GetGradeMultiplierAttribute(), bFound);
			return bFound ? Val : 1.0f;
		}
	}
	return 1.0f;
}

float AInstrumentBase::GetComboMultiplier() const
{
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(CurrentOwner))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			bool bFound;
			float Val = ASC->GetGameplayAttributeValue(URhythmScoreAttributeSet::GetComboMultiplierAttribute(), bFound);
			return bFound ? Val : 1.0f;
		}
	}
	return 1.0f;
}

void AInstrumentBase::HandleNoteDetected(ENoteResult InNoteResult)
{
	if (!IsOwnerLocallyControlled() || !ScoreData || InNoteResult == ENoteResult::Invalid || InNoteResult == ENoteResult::None) return;

	ADefaultPlayerState* PS = GetOwnerPlayerState();
	if (!PS) return;

	// 로컬 UI 업데이트
	if (!PS->HasAuthority())
	{
		PS->HandleCombo(InNoteResult);
	}
	// 서버에 콤보 업데이트
	PS->Server_HandleCombo(InNoteResult);

	int32 CurrentCombo = PS->GetComboData().CurrentCombo;
	float AddedScore = CalculateScore(InNoteResult, CurrentCombo);
	if (AddedScore > 0.f)
	{
		PS->Server_AddScore(FMath::RoundToInt(AddedScore));
	}
}

void AInstrumentBase::TryCreateIndicatorWidget()
{
	APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
	if (LocalPC && LocalPC->IsLocalController())
	{
		IndicatorWidgetInstance = CreateWidget<UOSI_WidgetBase>(LocalPC, IndicatorWidgetClass);
		if (IndicatorWidgetInstance.Get())
		{
			IndicatorWidgetInstance->TargetComponent = GetRootComponent();
			IndicatorWidgetInstance->AddToViewport();
			GetWorld()->GetTimerManager().ClearTimer(WidgetInitTimerHandle);
			TryUpdateIndicatorVisibility();
			return;
		}
	}
	GetWorld()->GetTimerManager().SetTimer(
		WidgetInitTimerHandle,
		this,
		&AInstrumentBase::TryCreateIndicatorWidget,
		0.1f,
		false
	);
}

bool AInstrumentBase::IsOwnerLocallyControlled() const
{
	const APawn* PawnOwner = Cast<APawn>(CurrentOwner);
	return (PawnOwner && PawnOwner->IsLocallyControlled());
}

ADefaultPlayerState* AInstrumentBase::GetOwnerPlayerState() const
{
	if (const APawn* PawnOwner = Cast<APawn>(CurrentOwner))
	{
		return PawnOwner->GetPlayerState<ADefaultPlayerState>();
	}
	return nullptr;
}


void AInstrumentBase::BindToRhythmSubsystem(bool bBind)
{
	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	if (URhythmSubsystem* RhythmSys = GameInstance ? GameInstance->GetSubsystem<URhythmSubsystem>() : nullptr)
	{
		if (bBind) RhythmSys->OnNoteDetected.AddDynamic(this, &ThisClass::HandleNoteDetected);
		else RhythmSys->OnNoteDetected.RemoveDynamic(this, &ThisClass::HandleNoteDetected);
	}
}



