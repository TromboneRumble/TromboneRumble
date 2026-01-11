// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/InstrumentViolin.h"
#include "Data/InstrumentScoreData.h"
#include "GameFramework/Character.h"
#include "Utilities/DebugHelper.h"

void AInstrumentViolin::OnRep_Equipped()
{
	Super::OnRep_Equipped();
	checkf(ViolinBodyMesh, TEXT("ViolinBodyMesh not valid Actor: %s"), *GetName());
	checkf(ViolinBowMesh, TEXT("ViolinBowMesh not valid Actor: %s"), *GetName());

	if (bIsEquipped)
	{
		SkeletalMeshComponent->SetSkeletalMesh(ViolinBowMesh);
		
		if (!ViolinBodyActor)
		{
			if (IsValid(ViolinBodyClass))
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = this;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				ViolinBodyActor = GetWorld()->SpawnActor<AActor>(ViolinBodyClass, GetActorTransform(), SpawnParams);

				if (ViolinBodyActor)
				{
					TArray<UPrimitiveComponent*> Comps;
					ViolinBodyActor->GetComponents(Comps);

					for (UPrimitiveComponent* Comp : Comps)
					{
						Comp->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
						Comp->SetReceivesDecals(false);
					}
				}
			}
		}
		if (IsValid(ViolinBodyActor) && CurrentOwner)
		{
			const ACharacter* OwnerChar = Cast<ACharacter>(CurrentOwner);
			if (OwnerChar)
			{
				ViolinBodyActor->SetActorHiddenInGame(false);
				ViolinBodyActor->AttachToComponent(
					OwnerChar->GetMesh(),
					FAttachmentTransformRules::SnapToTargetIncludingScale,
					FName(TEXT("socket_Violin"))
				);
			}
		}
	}
	else if (!bIsEquipped)
	{
		SkeletalMeshComponent->SetSkeletalMesh(ViolinBodyMesh);
		if (IsValid(ViolinBodyActor))
		{
			ViolinBodyActor->DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);
			ViolinBodyActor->SetActorHiddenInGame(true);
		}
		if (ActiveBuffHandle.IsValid())
		{
			BuffRemainingCount = 0;
			TotalNoteCount = 0;
			RemoveBuff();
		}
	}

	if (IsOwnerLocallyControlled())
	{
		TotalNoteCount = 0;
	}
}

float AInstrumentViolin::CalculateScore(ENoteResult InNoteResult, int32 CurrentCombo)
{
	if (InNoteResult == ENoteResult::Invalid || InNoteResult == ENoteResult::Bad || InNoteResult == ENoteResult::None)
	{
		TotalNoteCount = 0;
		if (ActiveBuffHandle.IsValid())
		{
			BuffRemainingCount = 0;
			RemoveBuff();
		}
		return 0.f;
	}
	TotalNoteCount++;

	// --- 버프 관리 로직 ---
	if (ActiveBuffHandle.IsValid())
	{
		// 이미 버프 중이라면 횟수 차감
		BuffRemainingCount--;
		if (BuffRemainingCount <= 0)
		{
			RemoveBuff(); // 횟수 소진 시 버프 해제
		}
		TotalNoteCount = 1;
	}
	else
	{
		// 버프가 없는 상태에서 발동 조건 확인 (N번째 노트마다)
		if (TotalNoteCount > 0 && (TotalNoteCount % ScoreData->ViolinBuffActivationCount == 0))
		{
			ApplyBuff(ScoreData->ViolinBuffEffectClass);
			BuffRemainingCount = ScoreData->ViolinBuffDurationCount;
		}
	}

	float BaseScore = (InNoteResult == ENoteResult::Excellent) ? ScoreData->PerfectScore : ScoreData->GoodScore;
	float GradeMultiplier = GetGradeMultiplier();


	// 등급 점수*배율 + 콤보 점수
	float FinalScore = (BaseScore * GradeMultiplier) + ScoreData->ComboBasePoint;

	if (IsOwnerLocallyControlled())
	{
		FString NoteResultStr = UEnum::GetValueAsString(InNoteResult); // Enum을 문자열로 변환

		FString DebugMsg = FString::Printf(
			TEXT("[Violin] Result: %s | (Base: %.0f * Mult:%.1f) + ComboBonus: %.0f = Final: %.0f"),
			*NoteResultStr,
			BaseScore,
			GradeMultiplier,
			ScoreData->ComboBasePoint,
			FinalScore
		);

		Debug::Print(DebugMsg);
	}


	return FinalScore;
}
