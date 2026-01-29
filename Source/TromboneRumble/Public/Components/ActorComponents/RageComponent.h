// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "Data/CharacterDataAsset.h"
#include "Utilities/Defines.h"
#include "RageComponent.generated.h"


class AWeaponBase;
class UGameplayEffect;
class UEquipmentComponent;
class AItemBase;

/// <summary>
/// TODO : InGameState에서는 분노 끄기
/// TODO : FaceMaterial을 HeadMaterial로 확장
/// </summary>
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TROMBONERUMBLE_API URageComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	URageComponent();

	void ActivateRage(bool bShouldRage) { isRageActive = bShouldRage; };

protected:
	virtual void BeginPlay() override;
	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void HandleEquipmentChanged(EEquipmentSlotType Slot, AItemBase* NewItem, AItemBase* OldItem);

	UFUNCTION()
	void OnRep_FillAmount();

	UPROPERTY(EditDefaultsOnly, Category = "Config|Buff|GAS")
	TSubclassOf<UGameplayEffect> RageBuffClass;

	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<AWeaponBase> DefaultWeaponClass;

	UPROPERTY(EditDefaultsOnly, Category = "Config|GAS")
	FGameplayTag SpeedMultiplierTag;

private:
	void UpdateSpeedEffect(float Multiplier);
	void RemoveRageBuff();
	bool IsAllEquipmentRemoved() const;

	UPROPERTY()
	UCharacterDataAsset* CharacterData;

	/** 리플리케이트되는 머티리얼 파라미터 값 */
	UPROPERTY(ReplicatedUsing = OnRep_FillAmount)
	float FillAmount = 0.0f;

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial;

	FActiveGameplayEffectHandle RageBuffHandle;

	bool bIsBuffActive = false;
	float BuffTimer = 0.0f;
	int32 LastAppliedMilestoneIndex = -1;

	bool isRageActive = false;
};
