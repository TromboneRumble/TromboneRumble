// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/InstrumentViolin.h"

#include "Characters/TromboneCharacterBase.h"
#include "Data/InstrumentScoreData.h"
#include "GameFramework/Character.h"
#include "UI/UserWidgets/Rhythm/ComboWidget/ViolinComboWidget.h"
#include "Utilities/DebugHelper.h"


void AInstrumentViolin::OnRep_CurrentOwner(AActor* OldActor)
{
	checkf(ViolinBodyMesh, TEXT("ViolinBodyMesh not valid Actor: %s"), *GetName());
	checkf(ViolinBowMesh, TEXT("ViolinBowMesh not valid Actor: %s"), *GetName());

	//Mesh 변경만 미리 한 후, Super::OnRep_CurrentOwner을 호출하여 Collision 재설정
	if (CurrentOwner)
	{
		SkeletalMeshComponent->SetSkeletalMesh(ViolinBowMesh);
	}
	else
	{
		SkeletalMeshComponent->SetSkeletalMesh(ViolinBodyMesh);
	}
	Super::OnRep_CurrentOwner(OldActor);

	if (CurrentOwner)
	{
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
		if (IsValid(ViolinBodyActor))
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

				// 로컬 플레이어가 든 경우에만 바이올린 본체도 X-Ray 실루엣에 포함
				if (IsOwnerLocallyControlled())
				{
					ATromboneCharacterBase::ApplyOccludedStencilToActor(ViolinBodyActor);
				}
				else
				{
					ATromboneCharacterBase::ClearOccludedStencilFromActor(ViolinBodyActor);
				}
			}
		}
	}
	else
	{

		if (IsValid(ViolinBodyActor))
		{
			ViolinBodyActor->DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);
			ViolinBodyActor->SetActorHiddenInGame(true);
			ATromboneCharacterBase::ClearOccludedStencilFromActor(ViolinBodyActor);
		}
		if (ActiveBuffHandle.IsValid())
		{
			BuffRemainingCount = 0;
			TotalNoteCount = 0;
			Server_RemoveBuff(CurrentOwner);
		}
	}

	if (IsOwnerLocallyControlled())
	{
		TotalNoteCount = 0;
	}
}

float AInstrumentViolin::CalculateScore(ENoteResult InNoteResult, int32 CurrentCombo)
{
	UViolinComboWidget* ViolinComboWidget = Cast<UViolinComboWidget>(ComboWidgetInstance.Get());

	if (InNoteResult == ENoteResult::Invalid || InNoteResult == ENoteResult::Bad || InNoteResult == ENoteResult::None)
	{
		TotalNoteCount = 0;
	}
	else
	{
		TotalNoteCount++;
	}
	

	// --- 버프 관리 로직 ---
	if (ActiveBuffHandle.IsValid())
	{
		// 이미 버프 중이라면 횟수 차감
		BuffRemainingCount--;
		if (BuffRemainingCount <= 0)
		{
			Server_RemoveBuff(CurrentOwner); // 횟수 소진 시 버프 해제
			TotalNoteCount = 0;
		}
		if (ViolinComboWidget)
		{
			ViolinComboWidget->SetPercentSmooth((float)BuffRemainingCount / float(ScoreData->ViolinBuffDurationCount));
		}
	}
	else
	{
		// 버프가 없는 상태에서 발동 조건 확인 (N번째 노트마다)
		if (TotalNoteCount > 0 && (TotalNoteCount % ScoreData->ViolinBuffActivationCount == 0))
		{
			Server_ApplyBuff(ScoreData->ViolinBuffEffectClass);
			BuffRemainingCount = ScoreData->ViolinBuffDurationCount;
		}
		if (ViolinComboWidget)
		{
			ViolinComboWidget->SetPercentSmooth(FMath::Min(1.0f, (float)TotalNoteCount / float(ScoreData->ViolinBuffActivationCount)));
		}
	}

	//점수 계산 로직 진행
	if (InNoteResult == ENoteResult::Invalid || InNoteResult == ENoteResult::Bad || InNoteResult == ENoteResult::None)
	{
		return 0.f;
	}

	float BaseScore = (InNoteResult == ENoteResult::Excellent) ? ScoreData->PerfectScore : ScoreData->GoodScore;
	float GradeMultiplier = GetGradeMultiplier();


	// 등급 점수*배율 + 콤보 점수
	float FinalScore = (BaseScore * GradeMultiplier) + ScoreData->ComboBasePoint;

	if (IsOwnerLocallyControlled())
	{
		//FString NoteResultStr = UEnum::GetValueAsString(InNoteResult); // Enum을 문자열로 변환

		//FString DebugMsg = FString::Printf(
		//	TEXT("[Violin] Result: %s | (Base: %.0f * Mult:%.1f) + ComboBonus: %.0f = Final: %.0f"),
		//	*NoteResultStr,
		//	BaseScore,
		//	GradeMultiplier,
		//	ScoreData->ComboBasePoint,
		//	FinalScore
		//);

		//Debug::Print(DebugMsg);
	}


	return FinalScore;
}
