// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/InstrumentBase.h"
#include "Subsystems/RhythmSubsystem.h"
#include "Framework/DefaultPlayerState.h"
#include "Data/InstrumentScoreData.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Data/RhythmScoreAttributeSet.h"
#include "Utilities/DebugHelper.h"

AInstrumentBase::AInstrumentBase()
{
}

void AInstrumentBase::OnRep_Equipped()
{
	Super::OnRep_Equipped();
	if (bIsEquipped)
	{
		if (IsOwnerLocallyControlled())
		{
			BindToRhythmSubsystem(true);
		}
	}
	else
	{
		RemoveBuff();
		BindToRhythmSubsystem(false);
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



