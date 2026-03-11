// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PodiumActor.generated.h"

class USkeletalMeshComponent;

UCLASS()
class TROMBONERUMBLE_API APodiumActor : public AActor
{
	GENERATED_BODY()
	
public:	
	APodiumActor();

	void ApplySkinColor(const FLinearColor& InSkinColor);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> MeshComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	int32 SkinMaterialIndex = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	int32 FaceMaterialIndex = 2;

private:
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> SkinMID;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> FaceMID;


};
