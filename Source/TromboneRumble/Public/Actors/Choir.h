// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/ChoirDataAsset.h"
#include "Choir.generated.h"

class UAnimSequence;

UCLASS()
class TROMBONERUMBLE_API AChoir : public AActor
{
	GENERATED_BODY()
	
public:
	AChoir();

	UFUNCTION(BlueprintCallable, Category = "Choir|Animation")
	void PlayIdle();

	UFUNCTION(BlueprintCallable, Category = "Choir|Animation")
	void PlaySing();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> MeshComponent;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> BookMesh;

private:
	void InitializeRandomAnimation();

	void StartAnimationInternal(FChoirAnimationSequence* InFaceSequence, UAnimSequence* InBodyAnim, bool bIsSinging);
	void ProcessNextFrame();
	void UpdateFaceMaterial(int32 Index);

	UPROPERTY(EditAnywhere, Category = "Config|Animation")
	TObjectPtr<UAnimSequence> IdleAnimAsset;

	UPROPERTY(EditAnywhere, Category = "Config|Animation")
	TObjectPtr<UAnimSequence> SingAnimAsset;

	UPROPERTY(EditAnywhere, Category = "Config|Data")
	TObjectPtr<UChoirDataAsset> AnimationData;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> FaceMID;

	float CurrentPlayRate = 1.0f;

	FChoirAnimationSequence* CurrentSequence = nullptr;
	int32 CurrentFrameIndex = 0;
	FTimerHandle TimerHandle_FaceAnim;
	FTimerHandle TimerHandle_StartDelay;

	UPROPERTY(EditAnywhere, Category = "Config")
	FName FaceExpressionParameterName = TEXT("ExpressionIndex");

	UPROPERTY(EditAnywhere, Category = "Config")
	int32 FaceMaterialIndex = 1;

	UPROPERTY(EditAnywhere, Category = "Config")
	FName BookSocketName = TEXT("Socket_Book");
};
