// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "Data/CharacterDataAsset.h"
#include "GameFramework/Character.h"
#include "Interfaces/CombatReceiver.h"
#include "TromboneCharacterBase.generated.h"

class UTromboneRagdollComponent;
class UAkAudioEvent;
class UAkComponent;
class UNiagaraComponent;
class UPhysicalAnimationComponent;
class UCharacterDataAsset;
class UCustomizationComponent;
class UMaterialInterface;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRagdollSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEndRagdollSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStunStateChanged, bool, bIsStunned);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInvincibleSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEndInvincibleSignature);

UCLASS()
class TROMBONERUMBLE_API ATromboneCharacterBase : public ACharacter, public ICombatReceiver
{
	GENERATED_BODY()

public:
	ATromboneCharacterBase();
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void OnRep_Controller() override;

	// ~ Begin ICombatReceiver Interfaces
	virtual void OnHitReceived_Implementation(const FHitData& HitData) override;
	// ~ End ICombatReceiver Interfaces
	
	void ApplySkinColor(const FLinearColor InSkinColor) const;
	void SetPlayerInput(const bool bShouldEnable);

	// 커스터마이징용 페이스 머티리얼 교체. nullptr 전달 시 원본 머티리얼로 복원
	void ApplyFaceMaterial(UMaterialInterface* Material);

	// X-Ray 실루엣용 CustomDepth stencil 값 설정 (단일 Primitive 컴포넌트)
	static void ApplyOccludedStencil(UPrimitiveComponent* Prim);
	// 지정 액터 내부의 모든 Primitive에만 stencil 적용 (자식 액터는 순회하지 않음)
	static void ApplyOccludedStencilToActor(AActor* Actor);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCustomizationComponent> CustomizationComp;

	FOnRagdollSignature OnRagdollDelegate;
	FEndRagdollSignature EndRagdollDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnStunStateChanged OnStunStateChanged;
	FOnInvincibleSignature OnInvincibleDelegate;
	FEndInvincibleSignature EndInvincibleDelegate;

protected:
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "Config|Data")
	TObjectPtr<UCharacterDataAsset> CharacterData;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Material")
	int32 SkinMaterialIndex = 1;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Material")
	int32 FaceMaterialIndex = 2;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Material")
	FName FaceExpressionParameterName = FName("ExpressionIndex");

	UPROPERTY(EditDefaultsOnly, Category = "Config|Sound")
	TObjectPtr<UAkAudioEvent> StunNiagaraSound;

	UPROPERTY(EditDefaultsOnly, Category = "Config|Components|Niagara")
	TObjectPtr<UNiagaraComponent> StunNiagaraComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Config|Sound")
	TObjectPtr<UAkAudioEvent> RagdollBooSound;


	UPROPERTY(EditAnywhere, Category = "Config|Components|Sound")
	TObjectPtr<UAkComponent> AkSoundComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Animation")
	TObjectPtr<UCurveVector> BounceCurve;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Components")
	TObjectPtr<UTromboneRagdollComponent> RagdollComponent;

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
	
	void DelayedSavePoseSnapshot();
	void InternalUnapplyRagdoll();
	bool IsFacingUp() const;
	float PoseSnapshotInterval = 0.1f;

	void UpdateSkinFromPlayerState();
	
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
	FTimerHandle TimerHandler_DelayedSavePostSnapshot;
	FTimerHandle TimerHandler_InternalUnapplyRagdoll;

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
	// BeginPlay에서 FaceMID 생성 직전 원본 머티리얼 캐싱 (커스터마이징 복원용)
	UPROPERTY()
	TObjectPtr<UMaterialInterface> OriginalFaceMaterial;
	UPROPERTY()
	TObjectPtr<UPhysicalAnimationComponent> PhysicalAnimationComp;

	FName PelvisBoneName = "pelvis";
	
	// ~ Begin Face Expression Region
	void PlayFaceSequence(ECharacterFaceState TargetState);
	void InternalPlayFaceSequence(const FCharacterFaceAnimationSequence* InSequence);
	void ExecuteFaceStep();
	void UpdateFaceExpression(ECharacterFaceType NewType);
	
	int32 CurrentSequenceStep = 0;
	FCharacterFaceAnimationSequence CurrentActiveSequence;
	FTimerHandle FaceSequenceTimerHandle;
	// ~ End Face Expression Region

	// ~ Bounce Character
	FTimeline BounceTimeline;
	void BoundBounceTimeline();
	UFUNCTION()
	void HandleBounceProgress(FVector Value);
	// ~ End Bounce Character
	
	int32 StunNiagaraPlayingID = 0;
	
public:
	UFUNCTION(Server, Reliable)
	void Server_DebugStun();

	UFUNCTION(Server, Reliable)
	void Server_DebugRagdoll();

public:
	//~ Begin Setter
	bool IsStun() const { return bIsStun; }
	bool IsRagdoll() const { return bIsRagdoll; }
	bool IsCanProcessInput() const { return bIsCanProcessInput; }
	UCharacterDataAsset* GetCharacterDataAsset() const { return CharacterData; }
	//~ End Setter
};