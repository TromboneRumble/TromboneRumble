// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/InstrumentCymbals.h"
#include "Data/InstrumentScoreData.h"
#include "Framework/DefaultPlayerState.h"
#include "GameFramework/Character.h"
#include "Utilities/DebugHelper.h"

void AInstrumentCymbals::Multicast_OnHitSuccess_Implementation(AActor* HitActor)
{
	Super::Multicast_OnHitSuccess_Implementation(HitActor);
	PlayHitSound();
}

void AInstrumentCymbals::Client_OnHitSuccess_Implementation(AActor* HitActor)
{
	Super::Client_OnHitSuccess_Implementation(HitActor);
	if (ADefaultPlayerState* DefaultPlayerState = GetOwnerPlayerState())
	{
		if (IsOwnerLocallyControlled())
		{
			FString DebugMsg = FString::Printf(
				TEXT("[Cymbals] HitResult: HitScore: %.0f"),
				ScoreData->AttackScore
			);
			Debug::Print(DebugMsg);
		}

		DefaultPlayerState->Server_AddScore(FMath::RoundToInt(ScoreData->AttackScore));
	}
}

void AInstrumentCymbals::OnRep_CurrentOwner(AActor* OldActor)
{
	Super::OnRep_CurrentOwner(OldActor);

	if (CurrentOwner)
	{
		if (!CymbalsRightHandActor)
		{
			if (IsValid(CymbalsHalfClass))
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = this;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				CymbalsRightHandActor = GetWorld()->SpawnActor<AActor>(CymbalsHalfClass, GetActorTransform(), SpawnParams);

				if (CymbalsRightHandActor)
				{
					TArray<UPrimitiveComponent*> Comps;
					CymbalsRightHandActor->GetComponents(Comps);

					for (UPrimitiveComponent* Comp : Comps)
					{
						Comp->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
						Comp->SetReceivesDecals(false);
					}
				}
			}
		}
		if (IsValid(CymbalsRightHandActor))
		{
			const ACharacter* OwnerChar = Cast<ACharacter>(CurrentOwner);
			if (OwnerChar)
			{
				CymbalsRightHandActor->SetActorHiddenInGame(false);
				CymbalsRightHandActor->AttachToComponent(
					OwnerChar->GetMesh(),
					FAttachmentTransformRules::SnapToTargetIncludingScale,
					FName(TEXT("socket_Cymbal_r"))
				);
			}
		}
	}
	else
	{
		if (IsValid(CymbalsRightHandActor))
		{
			CymbalsRightHandActor->DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);
			CymbalsRightHandActor->SetActorHiddenInGame(true);
		}
	}
}

float AInstrumentCymbals::CalculateScore(ENoteResult InNoteResult, int32 CurrentCombo)
{
	if (InNoteResult == ENoteResult::Invalid || InNoteResult == ENoteResult::Bad || InNoteResult == ENoteResult::None) return 0.f;

	float BaseScore = (InNoteResult == ENoteResult::Excellent) ? ScoreData->PerfectScore : ScoreData->GoodScore;

	// 등급 점수 + 콤보 점수
	float FinalScore = BaseScore + ScoreData->ComboBasePoint;

	if (IsOwnerLocallyControlled())
	{
		FString NoteResultStr = UEnum::GetValueAsString(InNoteResult); // Enum을 문자열로 변환

		FString DebugMsg = FString::Printf(
			TEXT("[Cymbals] Result: %s | Base: %.0f + ComboBonus: %.0f = Final: %.0f"),
			*NoteResultStr,
			BaseScore,
			ScoreData->ComboBasePoint,
			FinalScore
		);
		Debug::Print(DebugMsg);
	}


	return FinalScore;
}
