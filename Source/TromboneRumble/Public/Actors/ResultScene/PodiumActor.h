// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/CharacterDataAsset.h"
#include "Data/CustomizationSaveData.h"
#include "Utilities/Defines.h"
#include "GameFramework/Actor.h"
#include "PodiumActor.generated.h"

class USkeletalMeshComponent;
class UWidgetComponent;
class UCharacterDataAsset;
class UCustomizationComponent;
class UMaterialInterface;

UCLASS()
class TROMBONERUMBLE_API APodiumActor : public AActor
{
	GENERATED_BODY()
	
public:	
	APodiumActor();

	void ApplySkinColor(const FLinearColor& InSkinColor);

	// 플레이어 커스터마이징(Antenna/Face/Costume)을 포디움에 적용. ApplySkinColor 이후에 호출할 것
	void ApplyCustomization(const FCustomizationSaveData& Data);

	// CustomizationComponent::ApplyFace가 호출 — head face 슬롯 머티리얼을 교체하고 MID 재생성
	void ApplyFaceMaterial(UMaterialInterface* Material);

	// CustomizationComponent가 leader 메시 / 피부색을 조회하기 위한 getter
	USkeletalMeshComponent* GetMeshComponent() const { return MeshComponent; }
	FLinearColor GetSkinColor() const { return CachedSkinColor; }

	void SetPlayerName(const FString& InName);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetNameWidgetVisibility(bool bVisible);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UWidgetComponent> NameWidgetComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCustomizationComponent> CustomizationComp;

	UPROPERTY(EditDefaultsOnly, Category = "Config|Data")
	TObjectPtr<UCharacterDataAsset> CharacterData;

	UPROPERTY(EditDefaultsOnly, Category = "Config|Material")
	FName FaceExpressionParameterName = FName("ExpressionIndex");

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bIsCrying = false;

private:
	// head 메시의 skin/face 슬롯 MID를 멱등하게 확보(없으면 생성). 호출 순서와 무관하게 안전.
	void EnsureHeadMaterialInstances();

	void PlayFaceSequence(ECharacterFaceState TargetState);
	void InternalPlayFaceSequence(const FCharacterFaceAnimationSequence* InSequence);
	void ExecuteFaceStep();
	void UpdateFaceExpression(ECharacterFaceType NewType);

	int32 CurrentSequenceStep = 0;
	FCharacterFaceAnimationSequence CurrentActiveSequence;
	FTimerHandle FaceSequenceTimerHandle;

	// 현재 표정 — 커마 Face 교체로 FaceMID 재생성 시 다시 적용해 표정이 풀리지 않게 함
	ECharacterFaceType CurrentFaceType = ECharacterFaceType::Blink0;

	// 적용된 피부색 — follower 파츠/Face 틴트가 참조
	FLinearColor CachedSkinColor = FLinearColor::Black;

	// 커마 Face가 없을 때 복원할 원본 face 머티리얼 (FaceMID 생성 전 캐싱)
	TWeakObjectPtr<UMaterialInterface> OriginalFaceMaterial;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> SkinMID;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> FaceMID;


};
