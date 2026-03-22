// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/CharacterDataAsset.h"
#include "GameFramework/Actor.h"
#include "PodiumActor.generated.h"

class USkeletalMeshComponent;
class UCharacterDataAsset;

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

	UPROPERTY(EditDefaultsOnly, Category = "Config|Data")
	TObjectPtr<UCharacterDataAsset> CharacterData;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	int32 SkinMaterialIndex = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	int32 FaceMaterialIndex = 2;

	UPROPERTY(EditDefaultsOnly, Category = "Config|Material")
	FName FaceExpressionParameterName = FName("ExpressionIndex");

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bIsCrying = false;

private:
	void PlayFaceSequence(ECharacterFaceState TargetState);
	void InternalPlayFaceSequence(const FCharacterFaceAnimationSequence* InSequence);
	void ExecuteFaceStep();
	void UpdateFaceExpression(ECharacterFaceType NewType);

	int32 CurrentSequenceStep = 0;
	FCharacterFaceAnimationSequence CurrentActiveSequence;
	FTimerHandle FaceSequenceTimerHandle;
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> SkinMID;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> FaceMID;


};
