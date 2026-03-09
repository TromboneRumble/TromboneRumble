// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/ResultScene/PodiumActor.h"

APodiumActor::APodiumActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
}

void APodiumActor::ApplySkinColor(const FLinearColor& InSkinColor)
{
	if (SkinMID) SkinMID->SetVectorParameterValue(TEXT("BaseColor"), InSkinColor);
	if (FaceMID) FaceMID->SetVectorParameterValue(TEXT("BaseColor"), InSkinColor);
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
}


