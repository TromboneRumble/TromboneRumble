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

	SetVisibility(false, false);
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

	SetVisibility(bShow, true);
	if (bShow)
	{
		if (URhythmSubsystem* RhythmSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<URhythmSubsystem>())
		{
			RhythmSubsystem->OnNoteDetected.AddDynamic(this, &ThisClass::OnNoteDetectedHandler);
			RhythmSubsystem->OnInstrumentPicked.AddDynamic(this, &ThisClass::OnInstrumentPickedHandler);
		}
	}
}

#if WITH_EDITOR
void URingHitBoxComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	EnsureMID();
	ApplyMaterialParams();

	bHasBaseColor = false;
	CacheBaseColorIfNeeded();

	MarkRenderStateDirty();
}
#endif

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

void URingHitBoxComponent::OnInstrumentPickedHandler(EInstrumentType PrevType, EInstrumentType NewType)
{
	const APawn* PawnOwner = Cast<APawn>(GetOwner());
	const bool bShow = PawnOwner && PawnOwner->IsLocallyControlled();
	if (PawnOwner && bShow)
	{
		if (NewType == EInstrumentType::Background || EInstrumentType::None<=NewType)
		{
			SetVisibility(false, true);
		}
		else
		{
			SetVisibility(true, true);
		}
	}
}

void URingHitBoxComponent::EnsureMID()
{
	// 이미 만들어져 있으면 재사용
	if (RingMID) return;

	// Element 0에 이미 Material/MI가 들어있으면 Source Material 안 넣어도 됨
	RingMID = CreateDynamicMaterialInstance(0);

	//UMaterialInterface* OriginMat = RingMatOrigin.Get();
	//if (!OriginMat)
	//{
	//	RingMID = nullptr;
	//	CachedParentMat = nullptr;
	//	return;
	//}

	//// 슬롯 0에 MID가 이미 꽂혀있는 경우
	//if (RingMID && CachedParentMat.Get() == OriginMat)
	//{
	//	if (GetMaterial(0) == RingMID)
	//	{
	//		return;
	//	}

	//	// 누가 슬롯 0을 바꿔버린 경우: 다시 꽂아 복구
	//	SetMaterial(0, RingMID);
	//	return;
	//}

	//RingMID = CreateAndSetMaterialInstanceDynamicFromMaterial(0, OriginMat);
	//CachedParentMat = OriginMat;
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
