// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/StaticMeshComponents/RingHitBoxComponent.h"
#include "GameFramework/Pawn.h"


URingHitBoxComponent::URingHitBoxComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	CanCharacterStepUpOn = ECB_No;
	bReceivesDecals = false;
	SetCastShadow(false);

	SetVisibility(false, true);
	SetHiddenInGame(true, true);
}

void URingHitBoxComponent::OnRegister()
{
	Super::OnRegister();

	EnsureMID();
	ApplyMaterialParams();
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
}

void URingHitBoxComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	EnsureMID();
	ApplyMaterialParams();
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
