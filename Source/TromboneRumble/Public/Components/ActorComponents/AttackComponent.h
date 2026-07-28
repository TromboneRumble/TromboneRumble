// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TimerHandle.h"
#include "AttackComponent.generated.h"

enum class EWeaponType : uint8;
class AWeaponBase;
enum class EEquipmentSlotType : uint8;
class AItemBase;
class UCapsuleComponent;
class UCharacterAnimInstance;
class UWeaponDataAsset;

UCLASS()
class TROMBONERUMBLE_API UAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	UAttackComponent();
	
	void Attack();

private:
	
	UFUNCTION(Server, Reliable)
	void Server_ExecuteAttack();

	UFUNCTION(Server, Reliable)
	void Server_ExecuteAttackEnd();

	/** 공격 애니메이션 재생 (본인 제외 모두) */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAttackEffects();
	
	UFUNCTION(Client, Reliable)
	void Client_OnAttackRejected();
	
	/** 공격 애니메이션 재생 (이 머신에서) */
	void PlayAttackEffects() const;
	
	void UpdateAttackDelegateBinding(const bool bIsAttack);

	/** @return 몽타주가 공격 몽타주인지 */
	bool IsAttackMontage(const UAnimMontage* Montage) const;

	void HandleServerAttackFailsafe();

	/** @return 로컬 공격 예측이 아직 유효한지 (공격 연출 재생 중) */
	bool IsLocalAttackPredicted() const;

	/** @return WeaponType 의 공격 몽타주의 실제 재생 시간. 없으면 0 */
	float GetAttackMontagePlayTime(EWeaponType WeaponType) const;
	
	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	UFUNCTION()
	void HandleOnEquipmentChanged(EEquipmentSlotType Slot, AItemBase* NewItem, AItemBase* OldItem);

private:
	
	UPROPERTY(EditDefaultsOnly, Category = "AttackComponent")
	FName HeadSocketName = FName("head");
	
	UPROPERTY(EditDefaultsOnly, Category = "AttackComponent")
	TEnumAsByte<ECollisionChannel> AttackTraceChannel = ECC_GameTraceChannel1;
	
	UPROPERTY(EditDefaultsOnly, Category = "AttackComponent")
	TMap<EWeaponType, TObjectPtr<UAnimMontage>> AttackMontageMap;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<ACharacter> OwnerCharacter = nullptr;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCharacterAnimInstance> CharacterAnimInstance = nullptr;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<AWeaponBase> CurrentWeapon = nullptr;
	
	UPROPERTY(Transient)
	TObjectPtr<AWeaponBase> DefaultWeaponInstance = nullptr;

	/** 공격 진행 중 상태 (서버 권위) */
	UPROPERTY(Replicated)
	bool bAttackInProgress = false;

	/** 로컬 공격 연출의 만료 시각 */
	float LocalAttackPredictedUntilSeconds = 0.f;
	
	/** 공격 진행 중 상태 강제 종료 타이머 */
	FTimerHandle TimerHandle_ServerAttackFailsafe;

public:
	
	// ~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// ~ End UActorComponent Interface
	
	// ~ Begin Getters / Setters
	FORCEINLINE void SetDefaultWeaponInstance(AWeaponBase* DefaultWeaponInst) { DefaultWeaponInstance = DefaultWeaponInst; }
	FORCEINLINE TObjectPtr<AWeaponBase> GetCurrentWeapon() const { return CurrentWeapon; }
	// ~ End Getters / Setters
	
};