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
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStunSignature);

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
	FOnStunSignature OnStunDelegate;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<UCharacterDataAsset> CharacterData;

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

	void InternalUnapplyRagdoll();
	bool IsFacingUp() const;
	void RagdollUpdate();
	void SetActorLocationAndRotationDuringRagdoll();

	// Replication Notifies
	UFUNCTION()
	void OnRep_IsRagdoll();
	UFUNCTION()
	void OnRep_IsStun();
	UFUNCTION()
	void OnRep_SkinColor();
	// ~Replication Notifies

	FTimerHandle OnHitTimerHandle;

	bool bIsCanProcessInput = true;
	
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

	FName PelvisBoneName = "pelvis";

public:
	//~ Begin Setter
	bool IsStun() const { return bIsStun; }
	bool IsRagdoll() const { return bIsRagdoll; }
	bool IsCanProcessInput() const { return bIsCanProcessInput; }
	//~ End Setter
};