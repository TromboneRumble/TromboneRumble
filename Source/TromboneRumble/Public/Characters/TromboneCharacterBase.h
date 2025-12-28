// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/CombatReceiver.h"
#include "TromboneCharacterBase.generated.h"

class UPhysicalAnimationComponent;
class UCharacterDataAsset;
class UInputComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRagdollSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEndRagdollSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStunSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEndStunSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInvincibleSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEndInvincibleSignature);

UCLASS()
class TROMBONERUMBLE_API ATromboneCharacterBase : public ACharacter, public ICombatReceiver
{
	GENERATED_BODY()

public:
	ATromboneCharacterBase();
	
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnRep_PlayerState() override;

	// ~ Begin ICombatReceiver Interfaces
	virtual void OnHitReceived(const FHitData& HitData) override;
	// ~ End ICombatReceiver Interfaces
	
	void ApplySkinColor(const FLinearColor InSkinColor) const;
	void SetPlayerInput(const bool bShouldEnable);

	FOnRagdollSignature OnRagdollDelegate;
	FEndRagdollSignature EndRagdollDelegate;
	FOnStunSignature OnStunDelegate;
	FEndStunSignature EndStunDelegate;
	FOnInvincibleSignature OnInvincibleDelegate;
	FEndInvincibleSignature EndInvincibleDelegate;

protected:
	/** 눈 깜빡임 간격 범위 최소 값 */
	UPROPERTY(EditDefaultsOnly, Category = "Config|Eye Blinking", meta = (ClampMin = "0.1", ClampMax = "10.0", DisplayName = "눈 깜빡임 간격 최소값"))
	float EyeBlinkingIntervalMin = 2.0f;
	/** 눈 깜빡임 간격 범위 최대 값 */
	UPROPERTY(EditDefaultsOnly, Category = "Config|Eye Blinking", meta = (ClampMin = "0.1", ClampMax = "10.0", DisplayName = "눈 깜빡임 간격 최대값"))
	float EyeBlinkingIntervalMax = 5.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Config|Data")
	TObjectPtr<UCharacterDataAsset> CharacterData;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Material")
	int32 SkinMaterialIndex = 1;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Material")
	int32 FaceMaterialIndex = 2;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Material")
	FName FaceExpressionParameterName = FName("ExpressionIndex");
	
	UPROPERTY(VisibleAnywhere)
	EFaceExpressionType CurrentExpressionType = EFaceExpressionType::None;
	
private:
	void InitCharacter();
	void SetupCapsuleComponent();
	void SetupSkeletalMeshComponent();
	void SetupCharacterData() const;

	void OnRagdoll();
	void EndRagdoll();
	void OnStun();
	void EndStun();

	void ApplyStun();
	void UnapplyStun();
	
	void ApplyRagdoll();
	void UnapplyRagdoll();

	void UpdateSkinFromPlayerState();
	void UpdateFaceExpression(EFaceExpressionType NewType);
	void StartBlinking();
	void ExecuteBlinkStep();

	void InternalUnapplyRagdoll();
	bool IsFacingUp() const;
	void RagdollUpdate();
	void SetActorLocationAndRotationDuringRagdoll();

	void ApplyFlagPhysics();

	// Replication Notifies
	UFUNCTION()
	void OnRep_IsRagdoll();
	UFUNCTION()
	void OnRep_IsStun();
	UFUNCTION()
	void OnRep_IsInvincible();
	UFUNCTION()
	void OnRep_SkinColor();
	// ~Replication Notifies

	FTimerHandle OnHitTimerHandle;
	FTimerHandle InvincibilityTimerHandle;

	bool bIsCanProcessInput = true;
	
	UPROPERTY(ReplicatedUsing = OnRep_IsInvincible)
	bool bIsInvincible = false;
	UPROPERTY(ReplicatedUsing = OnRep_IsRagdoll)
	bool bIsRagdoll = false;
	UPROPERTY(ReplicatedUsing = OnRep_IsStun)
	bool bIsStun = false;
	UPROPERTY(ReplicatedUsing = OnRep_SkinColor)
	FLinearColor SkinColor = FLinearColor::Black;
	
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> SkinMID;
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> FaceMID;
	UPROPERTY()
	TObjectPtr<UPhysicalAnimationComponent> PhysicalAnimationComp;

	FName PelvisBoneName = "pelvis";
	
	FTimerHandle BlinkingTimerHandle;
	FTimerHandle BlinkStepTimerHandle;
	int32 BlinkStep = 0;

public:
	//~ Begin Setter
	bool IsStun() const { return bIsStun; }
	bool IsRagdoll() const { return bIsRagdoll; }
	bool IsCanProcessInput() const { return bIsCanProcessInput; }
	//~ End Setter
};