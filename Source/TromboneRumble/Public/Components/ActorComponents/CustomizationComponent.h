// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/CustomizationPartRow.h"
#include "Data/CustomizationSaveData.h"
#include "Utilities/Defines.h"
#include "CustomizationComponent.generated.h"

class UStaticMeshComponent;
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

protected:
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;

private:
	void ApplySlot(ECustomizationSlotType Slot, FName Key);
	void ApplyAntenna(const FCustomizationPartRow* Row);
	void ApplyFace(const FCustomizationPartRow* Row);
	void ApplyCostume(const FCustomizationPartRow* Row);

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
	TObjectPtr<UStaticMeshComponent> AntennaComp;
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> CostumeComp;

	static const FName AntennaSocketName;
	static const FName CostumeSocketName;
};
