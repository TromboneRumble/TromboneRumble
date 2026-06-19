// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/InstrumentCymbals.h"

#include "Characters/DefaultTromboneCharacter.h"
#include "Data/InstrumentScoreData.h"
#include "Framework/DefaultPlayerState.h"
#include "GameFramework/Character.h"
#include "Utilities/DebugHelper.h"


void AInstrumentCymbals::Client_OnHitSuccess_Implementation(AActor* HitActor)
{
	Super::Client_OnHitSuccess_Implementation(HitActor);
	if (ADefaultPlayerState* DefaultPlayerState = GetOwnerPlayerState())
	{
		if (ADefaultTromboneCharacter* TromboneCharacter = Cast<ADefaultTromboneCharacter>(HitActor))
		{
			if (!TromboneCharacter->IsRagdoll() && !TromboneCharacter->IsStun() && IsOwnerLocallyControlled())
			{
				FString DebugMsg = FString::Printf(
					TEXT("[Cymbals] HitResult: HitScore: %.0f"),
					ScoreData->AttackScore
				);
				Debug::Print(DebugMsg);
				DefaultPlayerState->AddScore(FMath::RoundToInt(ScoreData->AttackScore), EScoreType::CymbalsHit);
			}
		}
		
	}
}

void AInstrumentCymbals::OnRep_CurrentOwner(AActor* OldActor)
{
	Super::OnRep_CurrentOwner(OldActor);

	if (CurrentOwner)
	{
		SkeletalMeshComponent->SetSkeletalMesh(CymbalsHalfMesh);

		if (!CymbalsHalfActor)
		{
			if (IsValid(CymbalsHalfClass))
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = this;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				CymbalsHalfActor = GetWorld()->SpawnActor<AActor>(CymbalsHalfClass, GetActorTransform(), SpawnParams);

				if (CymbalsHalfActor)
				{
					TArray<UPrimitiveComponent*> Comps;
					CymbalsHalfActor->GetComponents(Comps);

					for (UPrimitiveComponent* Comp : Comps)
					{
						Comp->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
						Comp->SetReceivesDecals(false);
					}
				}
			}
		}
		if (IsValid(CymbalsHalfActor))
		{
			const ACharacter* OwnerChar = Cast<ACharacter>(CurrentOwner);
			if (OwnerChar)
			{
				CymbalsHalfActor->SetActorHiddenInGame(false);
				CymbalsHalfActor->AttachToComponent(
					OwnerChar->GetMesh(),
					FAttachmentTransformRules::SnapToTargetIncludingScale,
					FName(TEXT("socket_Cymbal_r"))
				);

				// 로컬 플레이어가 든 경우에만 반쪽 심벌즈도 X-Ray 실루엣에 포함
				if (IsOwnerLocallyControlled())
				{
					ATromboneCharacterBase::ApplyOccludedStencilToActor(CymbalsHalfActor);
				}
				else
				{
					ATromboneCharacterBase::ClearOccludedStencilFromActor(CymbalsHalfActor);
				}
			}
		}
	}
	else
	{
		SkeletalMeshComponent->SetSkeletalMesh(CymbalsFullMesh);
		if (IsValid(CymbalsHalfActor))
		{
			CymbalsHalfActor->DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);
			CymbalsHalfActor->SetActorHiddenInGame(true);
			ATromboneCharacterBase::ClearOccludedStencilFromActor(CymbalsHalfActor);
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

		//FString DebugMsg = FString::Printf(
		//	TEXT("[Cymbals] Result: %s | Base: %.0f + ComboBonus: %.0f = Final: %.0f"),
		//	*NoteResultStr,
		//	BaseScore,
		//	ScoreData->ComboBasePoint,
		//	FinalScore
		//);
		//Debug::Print(DebugMsg);
	}


	return FinalScore;
}
