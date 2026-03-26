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
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
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

	if (URhythmSubsystem* RhythmSys = GetGameInstance()->GetSubsystem<URhythmSubsystem>())
	{
		RhythmSys->OnRhythmGameStateChanged.AddDynamic(this, &ThisClass::HandleRhythmGameStateChanged);
	}
}

void AInstrumentBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
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
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(WidgetInitTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(IndicatorRetryTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void AInstrumentBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AInstrumentBase, ActiveBuffHandle);
}

void AInstrumentBase::Client_OnHitSuccess_Implementation(AActor* HitActor)
{
	Super::Client_OnHitSuccess_Implementation(HitActor);

	if (const APawn* PawnOwner = Cast<APawn>(CurrentOwner))
	{
		if (ADefaultPlayerState* DefaultPlayerState = PawnOwner->GetPlayerState<ADefaultPlayerState>())
		{
			if (ADefaultTromboneCharacter* TromboneCharacter = Cast<ADefaultTromboneCharacter>(HitActor))
			{
				if (!TromboneCharacter->IsRagdoll() && !TromboneCharacter->IsStun() && IsOwnerLocallyControlled())
				{
					DefaultPlayerState->AddScore(FMath::RoundToInt(ScoreData->AttackScore), EScoreType::OnHit);
				}
			}
		}
	}
}

void AInstrumentBase::OnRep_CurrentOwner(AActor* OldActor)
{
	Super::OnRep_CurrentOwner(OldActor);
	if (CurrentOwner)
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
		BindToRhythmSubsystem(false);
		const APawn* PawnOwner = Cast<APawn>(OldActor);
		if (PawnOwner && PawnOwner->IsLocallyControlled())
		{
			if (ADefaultTromboneCharacter* TromboneCharacter = Cast<ADefaultTromboneCharacter>(OldActor))
			{
				UWidgetComponent* WidgetComponent = TromboneCharacter->GetComboWidgetComponent();
				if (ComboWidgetClass && WidgetComponent->GetWidgetClass() == ComboWidgetClass)
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

void AInstrumentBase::Unequip(AActor* OwnerActor)
{
	if (!HasAuthority() || !CurrentOwner) return;
	Server_RemoveBuff(CurrentOwner);
	Super::Unequip(OwnerActor);
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

	if (CurrentOwner)
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

void AInstrumentBase::HandleRhythmGameStateChanged(ERhythmGameState RhythmGameState)
{
	if (RhythmGameState == ERhythmGameState::Ended)
	{
		if (IndicatorWidgetInstance.Get())
		{
			IndicatorWidgetInstance->RemoveFromParent();
			IndicatorWidgetInstance = nullptr;
		}
	}
}

void AInstrumentBase::Server_ApplyBuff_Implementation(TSubclassOf<UGameplayEffect> BuffClass)
{
	if (!HasAuthority()) return;
	if (!BuffClass || ActiveBuffHandle.IsValid() || !CurrentOwner) return;
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(CurrentOwner))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			Context.AddSourceObject(this);
			ActiveBuffHandle = ASC->ApplyGameplayEffectToSelf(BuffClass->GetDefaultObject<UGameplayEffect>(), 1.f, Context);
			OnRep_ActiveBuffHandle();
		}
	}
}

void AInstrumentBase::OnRep_ActiveBuffHandle()
{
	if (!IsOwnerLocallyControlled()) return;
	if (ActiveBuffHandle.IsValid())
	{
		if (BuffActivationSound)
		{
			UAkGameplayStatics::PostEvent(BuffActivationSound, this, 0, FOnAkPostEventCallback());
		}
		OnBuffStateChanged.Broadcast(true);

		FString BuffName = TEXT("Unknown Buff");
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(CurrentOwner))
		{
			if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
			{
				const UGameplayEffect* GEDef = ASC->GetGameplayEffectDefForHandle(ActiveBuffHandle);
				if (GEDef)
				{
					BuffName = GEDef->GetName();
				}
			}
		}
		FString DebugMsg = FString::Printf(TEXT(">>> [Buff ON] Handle ID: %s | Effect: %s Applied!"),
			*ActiveBuffHandle.ToString(),
			*BuffName
		);
		Debug::Print(DebugMsg);
	}
	else
	{
		OnBuffStateChanged.Broadcast(false);
		Debug::Print(TEXT("<<< [Buff OFF] Buff Removed"));
	}
}

void AInstrumentBase::Server_RemoveBuff_Implementation(AActor* InActor)
{
	if (ActiveBuffHandle.IsValid() && InActor)
	{
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(InActor))
		{
			if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
			{
				ASC->RemoveActiveGameplayEffect(ActiveBuffHandle);
			}
		}
	}
	ActiveBuffHandle.Invalidate();
	OnBuffStateChanged.Broadcast(false);
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

	//Calculate Score
	PS->HandleCombo(InNoteResult);

	int32 CurrentCombo = PS->GetCurrentCombo();
	float AddedScore = CalculateScore(InNoteResult, CurrentCombo);
	if (AddedScore > 0.f)
	{
		if (ActiveBuffHandle.IsValid())
		{
			switch (InstrumentType)
			{
			case EInstrumentType::Trombone:
				PS->AddScore(FMath::RoundToInt(AddedScore), EScoreType::BuffedTromboneScore);
				break;
			case EInstrumentType::Violin:
				PS->AddScore(FMath::RoundToInt(AddedScore), EScoreType::BuffedViolinScore);
				break;
			}
		}
		else
		{
			PS->AddScore(FMath::RoundToInt(AddedScore), EScoreType::RhythmScore);
		}
		
	}
	//~Calculate Score

	if (PerfectNoteHitSound && InNoteResult == ENoteResult::Excellent)
	{
		UAkGameplayStatics::PostEvent(PerfectNoteHitSound, this, 0, FOnAkPostEventCallback());
	}
	else if (GoodNoteHitSound && InNoteResult == ENoteResult::Good)
	{
		UAkGameplayStatics::PostEvent(GoodNoteHitSound, this, 0, FOnAkPostEventCallback());
	}
}

void AInstrumentBase::TryCreateIndicatorWidget()
{
	if (!IsValid(this)) return;
	GetWorld()->GetTimerManager().ClearTimer(WidgetInitTimerHandle);


	APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
	if (LocalPC && LocalPC->IsLocalController())
	{
		IndicatorWidgetInstance = CreateWidget<UOSI_WidgetBase>(LocalPC, IndicatorWidgetClass);
		if (IndicatorWidgetInstance.Get())
		{
			IndicatorWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
			IndicatorWidgetInstance->TargetComponent = GetRootComponent();
			//WBP_Rhythm보다 한칸 아래
			IndicatorWidgetInstance->AddToViewport(-1);
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



