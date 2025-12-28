// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/StaticMeshComponents/RingHitBoxComponent.h"
#include "Subsystems/RhythmSubsystem.h"
#include "GameFramework/Pawn.h"


URingHitBoxComponent::URingHitBoxComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	CanCharacterStepUpOn = ECB_No;
	bReceivesDecals = false;
	SetCastShadow(false);

	SetVisibility(true, true);
	SetHiddenInGame(true, true);
}

void URingHitBoxComponent::OnRegister()
{
	Super::OnRegister();

	EnsureMID();
	ApplyMaterialParams();

	bHasBaseColor = false;
	CacheBaseColorIfNeeded();
}

void URingHitBoxComponent::BeginPlay()
{
	Super::BeginPlay();

	EnsureMID();
	ApplyMaterialParams();

	//멀티플레이 환경에서 자신만 볼수있게
	const APawn* PawnOwner = Cast<APawn>(GetOwner());
	const bool bShow = PawnOwner && PawnOwner->IsLocallyControlled();

	SetHiddenInGame(!bShow, true);
	SetVisibility(bShow, true);
	if (bShow)
	{
		if (URhythmSubsystem* RhythmSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<URhythmSubsystem>())
		{
			RhythmSubsystem->OnNoteDetected.AddDynamic(this, &ThisClass::OnNoteDetectedHandler);
		}
	}
}

void URingHitBoxComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	EnsureMID();
	ApplyMaterialParams();

	bHasBaseColor = false;
	CacheBaseColorIfNeeded();

	MarkRenderStateDirty();
}

void URingHitBoxComponent::OnNoteDetectedHandler(ENoteResult InNoteResult)
{
	switch (InNoteResult)
	{
	case ENoteResult::Bad:
		FlashToColor(BadFlashColor);
		break;
	case ENoteResult::Good:
		FlashToColor(GoodFlashColor);
		break;
	case ENoteResult::Excellent:
		FlashToColor(ExcellentFlashColor);
		break;
	default:
		break;
	}
	
	
}

void URingHitBoxComponent::EnsureMID()
{
	UMaterialInterface* CurrentMat = GetMaterial(0);
	if (!CurrentMat)
	{
		RingMID = nullptr;
		CachedParentMat = nullptr;
		return;
	}

	// 슬롯 0에 MID가 이미 꽂혀있는 경우
	if (UMaterialInstanceDynamic* CurrentMID = Cast<UMaterialInstanceDynamic>(CurrentMat))
	{
		UMaterialInterface* ParentMat = CurrentMID->Parent.Get();
		if (!ParentMat)
		{
			ParentMat = CurrentMat; 
		}

		// 이미 우리가 관리하는 MID면 그대로
		if (RingMID == CurrentMID && CachedParentMat.Get() == ParentMat)
		{
			return;
		}

		// 슬롯에 꽂힌 MID를 채택
		RingMID = CurrentMID;
		CachedParentMat = ParentMat;
		return;
	}

	// 슬롯 0이 Material/MIC 등(MID 아님) -> 자동으로 MID 생성해서 꽂기
	if (!RingMID || CachedParentMat.Get() != CurrentMat)
	{
		RingMID = CreateAndSetMaterialInstanceDynamicFromMaterial(0, CurrentMat);
		CachedParentMat = CurrentMat;
	}
}

void URingHitBoxComponent::ApplyMaterialParams()
{
	if (!RingMID)
	{
		UE_LOG(LogTemp, Warning, TEXT("No RingMID"));
		return;
	}
	RingMID->SetScalarParameterValue(TEXT("StartOuterRadius"), StartOuterRadius);
	RingMID->SetScalarParameterValue(TEXT("StartInnerRadius"), StartInnerRadius);
	RingMID->SetScalarParameterValue(TEXT("EndOuterRadius"), EndOuterRadius);
	RingMID->SetScalarParameterValue(TEXT("EndInnerRadius"), EndInnerRadius);
	RingMID->SetScalarParameterValue(TEXT("SizeAlpha"), SizeAlpha);
	RingMID->SetScalarParameterValue(TEXT("FadePercent"), FadePercent);
}

void URingHitBoxComponent::CacheBaseColorIfNeeded()
{
	if (bHasBaseColor)
	{
		return;
	}

	EnsureMID();
	if (!RingMID)
	{
		return;
	}

	// 머티리얼에서 현재 색을 읽어 원본색으로 사용
	FLinearColor Current;
	if (RingMID->GetVectorParameterValue(ColorParamName, Current))
	{
		CachedBaseColor = Current;
	}
	else
	{
		CachedBaseColor = FLinearColor::White;
	}

	bHasBaseColor = true;
}

void URingHitBoxComponent::FlashToColor(const FLinearColor& InColor)
{
	EnsureMID();
	if (!RingMID)
	{
		return;
	}

	CacheBaseColorIfNeeded();

	// 기존 복귀 타이머 제거(연타 대응)
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FlashTimerHandle);
	}

	RingMID->SetVectorParameterValue(ColorParamName, InColor);
	
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			FlashTimerHandle,
			this,
			&URingHitBoxComponent::RestoreBaseColor,
			FlashDuration,
			false
		);
	}
}

void URingHitBoxComponent::RestoreBaseColor()
{
	if (!RingMID)
	{
		return;
	}

	RingMID->SetVectorParameterValue(ColorParamName, CachedBaseColor);
}
