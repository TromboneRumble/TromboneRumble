// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/CustomizationPartRow.h"
#include "Data/CustomizationSaveData.h"
#include "Utilities/Defines.h"
#include "CustomizationComponent.generated.h"

class USkeletalMeshComponent;
class UMaterialInstanceDynamic;
class UDataTable;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TROMBONERUMBLE_API UCustomizationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCustomizationComponent();

	// SaveData로부터 모든 슬롯 적용. NAME_None = Order 0(기본값) 사용
	void LoadFromSaveData(const FCustomizationSaveData& Data);

	// 현재 선택 상태를 SaveData 형식으로 반환
	FCustomizationSaveData GetCurrentSaveData() const;

	// 해당 슬롯을 Direction(+1 앞으로 / -1 뒤로) 방향으로 순환
	UFUNCTION(BlueprintCallable)
	void StepPart(ECustomizationSlotType Slot, int32 Direction);

	// 모든 슬롯을 무작위 선택
	UFUNCTION(BlueprintCallable)
	void RandomizeAll();

	// 지정한 슬롯을 특정 RowName Key로 직접 설정 (디버깅용)
	UFUNCTION(BlueprintCallable)
	void SetPartByKey(ECustomizationSlotType Slot, FName Key);

	// Original 대비 변경 여부 (Back 버튼 dirty 체크용)
	bool IsDirtyFrom(const FCustomizationSaveData& Original) const;

	// follower(costume/antenna) 메시의 skin 슬롯에 피부색 적용. 머리(leader)는 owner가 별도 처리
	void ApplyPartsSkinColor(const FLinearColor& InColor) const;

	// 파츠 스킨컬러 틴트 on/off. CustomizeMap(ACustomizePawn)은 false로 두어 기본 머티리얼 그대로 표시
	void SetApplyPartsSkinColor(bool bEnable) { bApplyPartsSkinColor = bEnable; }

	// 슬롯 이름으로 인덱스를 찾아 MID를 확보(없으면 생성). 슬롯이 없으면 nullptr.
	static UMaterialInstanceDynamic* EnsureSlotMID(USkeletalMeshComponent* Mesh, FName SlotName);

protected:
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;

private:
	void ApplySlot(ECustomizationSlotType Slot, FName Key);
	void ApplyAntenna(const FCustomizationPartRow* Row);
	void ApplyFace(const FCustomizationPartRow* Row);
	void ApplyCostume(const FCustomizationPartRow* Row);

	// costume/antenna 공통: 기존 컴포넌트 정리 → SkeletalMesh follower 생성 → LeaderPose 연결 → skin/스텐실 적용
	void ApplyFollowerPart(const FCustomizationPartRow* Row,
		TObjectPtr<USkeletalMeshComponent>& Comp,
		TObjectPtr<UMaterialInstanceDynamic>& SkinMID,
		const TCHAR* CompName);

	// owner 타입에 맞는 leader(메인) 스켈레탈 메시 반환. follower 오인을 막기 위해 명시적으로 결정
	USkeletalMeshComponent* ResolveLeaderMesh() const;

	// owner PlayerState의 현재 피부색 (없으면 Black)
	FLinearColor GetOwnerSkinColor() const;

	TArray<FName> GetSortedKeysForSlot(ECustomizationSlotType Slot) const;
	const FCustomizationPartRow* FindRowByKey(FName Key) const;
	FName FindDefaultKeyForSlot(ECustomizationSlotType Slot) const;

	// 런타임에 DataTable을 지정. Unreal Editor에서 Blueprint 기본값으로 할당
	UPROPERTY(EditDefaultsOnly, Category = "Customization")
	TSoftObjectPtr<UDataTable> CustomizationDataTable;

	UPROPERTY()
	TObjectPtr<UDataTable> CachedDataTable;

	FName CurrentAntennaKey = NAME_None;
	FName CurrentFaceKey    = NAME_None;
	FName CurrentCostumeKey = NAME_None;

	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> AntennaComp;
	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> CostumeComp;

	// follower 메시의 skin 슬롯 MID (피부색 동기화용)
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> AntennaSkinMID;
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> CostumeSkinMID;

	// false면 파츠에 스킨컬러를 입히지 않고 기본 머티리얼 그대로 사용 (CustomizeMap용)
	bool bApplyPartsSkinColor = true;
};
