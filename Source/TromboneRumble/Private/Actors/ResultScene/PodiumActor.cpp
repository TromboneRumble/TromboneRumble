// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/ResultScene/PodiumActor.h"
#include "Utilities/Defines.h"
#include "Components/WidgetComponent.h"
#include "UI/UserWidgets/InGame/PodiumNameWidget.h"

APodiumActor::APodiumActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	NameWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("NameWidgetComponent"));
	if (NameWidgetComponent)
	{
		NameWidgetComponent->SetupAttachment(RootComponent);
		NameWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
		NameWidgetComponent->SetDrawAtDesiredSize(true);
	}
	
}

void APodiumActor::ApplySkinColor(const FLinearColor& InSkinColor)
{
	if (SkinMID) SkinMID->SetVectorParameterValue(TEXT("BaseColor"), InSkinColor);
	if (FaceMID) FaceMID->SetVectorParameterValue(TEXT("BaseColor"), InSkinColor);
}

void APodiumActor::SetPlayerName(const FString& InName)
{
	if (NameWidgetComponent)
	{
		if (UPodiumNameWidget* NameWidget = Cast<UPodiumNameWidget>(NameWidgetComponent->GetUserWidgetObject()))
		{
			NameWidget->SetPlayerName(InName);
		}
	}
}

void APodiumActor::SetNameWidgetVisibility(bool bVisible)
{
	if (NameWidgetComponent)
	{
		NameWidgetComponent->SetVisibility(bVisible);
	}
}


void APodiumActor::BeginPlay()
{
	Super::BeginPlay();
	if (UMaterialInterface* CurrentSkinMat = MeshComponent->GetMaterial(SkinMaterialIndex))
	{
		SkinMID = Cast<UMaterialInstanceDynamic>(CurrentSkinMat);
		if (!SkinMID) SkinMID = MeshComponent->CreateAndSetMaterialInstanceDynamic(SkinMaterialIndex);
	}

	if (UMaterialInterface* CurrentFaceMat = MeshComponent->GetMaterial(FaceMaterialIndex))
	{
		FaceMID = Cast<UMaterialInstanceDynamic>(CurrentFaceMat);
		if (!FaceMID) FaceMID = MeshComponent->CreateAndSetMaterialInstanceDynamic(FaceMaterialIndex);
	}
	if (bIsCrying)
	{
		PlayFaceSequence(ECharacterFaceState::Cry);
	}

	if (NameWidgetComponent)
	{
		NameWidgetComponent->SetVisibility(false);
	}
}

void APodiumActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorldTimerManager().ClearTimer(FaceSequenceTimerHandle);
		FaceSequenceTimerHandle.Invalidate();
	}
	Super::EndPlay(EndPlayReason);
}

void APodiumActor::PlayFaceSequence(ECharacterFaceState TargetState)
{
	if (!CharacterData) return;

	if (const FCharacterFaceAnimationSequence* FaceAnimData = CharacterData->FaceSequences.Find(TargetState))
	{
		InternalPlayFaceSequence(FaceAnimData);
	}
}

void APodiumActor::InternalPlayFaceSequence(const FCharacterFaceAnimationSequence* InSequence)
{
	GetWorld()->GetTimerManager().ClearTimer(FaceSequenceTimerHandle);
	CurrentActiveSequence = *InSequence;
	CurrentSequenceStep = 0;
	ExecuteFaceStep();
}

void APodiumActor::ExecuteFaceStep()
{
	if (CurrentActiveSequence.Sequence.Num() == 0) return;

	UpdateFaceExpression(CurrentActiveSequence.Sequence[CurrentSequenceStep]);
	CurrentSequenceStep++;

	if (CurrentSequenceStep < CurrentActiveSequence.Sequence.Num())
	{
		GetWorld()->GetTimerManager().SetTimer(FaceSequenceTimerHandle, this, &ThisClass::ExecuteFaceStep, CurrentActiveSequence.Interval, false);
	}
	else if (CurrentActiveSequence.bLoop)
	{
		CurrentSequenceStep = 0;
		float NextDelay = FMath::FRandRange(CurrentActiveSequence.MinLoopDelay, CurrentActiveSequence.MaxLoopDelay);
		if (NextDelay <= 0.0f) NextDelay = CurrentActiveSequence.Interval;

		GetWorld()->GetTimerManager().SetTimer(FaceSequenceTimerHandle, this, &ThisClass::ExecuteFaceStep, NextDelay, false);
	}
}

void APodiumActor::UpdateFaceExpression(ECharacterFaceType NewType)
{
	if (FaceMID)
	{
		// Material의 ExpressionIndex 파라미터 업데이트
		FaceMID->SetScalarParameterValue(FaceExpressionParameterName, static_cast<float>(NewType));
	}
}


