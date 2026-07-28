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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStunStateChanged, bool, bIsStunned);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInvincibleSignature);

/** EInputBlockReason 
 * 여러 시스템(래그돌, 튜토리얼, 서버 등등)에서 입력을 차단할 수 있는데, 
 * 이를 비트 마스킹으로 관리하여 서로의 잠금을 덮어쓰지 않도록 함
 */
enum class EInputBlockReason : uint8
{
	None       = 0,
	ServerLock = 1 << 0,
	Ragdoll    = 1 << 1,
	Stun       = 1 << 2,
	Tutorial   = 1 << 3,
};

/** TODO : 온갖 기능이 다 들어있는 캐릭터 코드 정리하기 */
UCLASS()
class TROMBONERUMBLE_API ATromboneCharacterBase : public ACharacter, public ICombatReceiver
{
	GENERATED_BODY()

public:
	ATromboneCharacterBase();

	// ~ Begin ICombatReceiver Interfaces
	virtual void OnHitReceived_Implementation(const FHitData& HitData) override;
	// ~ End ICombatReceiver Interfaces
	
	virtual void ApplySkinColor(const FLinearColor InSkinColor) const;
	FLinearColor GetSkinColor() const { return SkinColor; }
	
	/** Add a reason for the input lock */
	void AddInputBlock(EInputBlockReason Reason);
	
	/** Remove a reason for the input lock */
	void RemoveInputBlock(EInputBlockReason Reason);

	/** ServerLock 비트를 리플리케이트해 각 머신의 비트 마스크에 적용. Server Only. */
	void Server_SetInputEnabled(const bool bEnable);

	// 커스터마이징용 페이스 머티리얼 교체. nullptr 전달 시 원본 머티리얼로 복원
	void ApplyFaceMaterial(UMaterialInterface* Material);

	// X-Ray 실루엣용 CustomDepth stencil 값 설정 (단일 Primitive 컴포넌트)
	static void ApplyOccludedStencil(UPrimitiveComponent* Prim);
	// X-Ray 실루엣용 CustomDepth 렌더 해제 (무기 드롭/원격 소유 시 등)
	static void ClearOccludedStencil(UPrimitiveComponent* Prim);
	// 지정 액터 내부의 모든 Primitive에만 stencil 적용 (자식 액터는 순회하지 않음)
	static void ApplyOccludedStencilToActor(AActor* Actor);
	// 지정 액터 내부의 모든 Primitive의 CustomDepth 렌더 해제
	static void ClearOccludedStencilFromActor(AActor* Actor);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCustomizationComponent> CustomizationComp;

	FOnStunStateChanged OnStunStateChanged;
	FInvincibleSignature OnInvincibleDelegate;
	FInvincibleSignature EndInvincibleDelegate;

protected:
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "Config|Data")
	TObjectPtr<UCharacterDataAsset> CharacterData;
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

	// false면 SkinColor를 skin/face MID에 틴트하지 않고 머티리얼 기본색 사용 (PlayerState 없는 더미용)
	bool bApplySkinColorTint = true;

private:
	void SetupCharacterData() const;

	void OnStun();
	void EndStun();

	void ApplyStun();
	void UnapplyStun();

	UFUNCTION()
	void HandleRagdollStarted();
	UFUNCTION()
	void HandleRagdollEnded();
	UFUNCTION()
	void HandleRagdollPhysicsEnabled();

	/** FHitData 를 최종 넉백 속도로 계산한다. 폭발이면 방사형, 아니면 수평 힘 + 수직 힘 조합 */
	FVector CalculateKnockbackVelocity(const FHitData& HitData) const;

	UFUNCTION(Client, Reliable)
	void Client_ApplyKnockback(FVector KnockbackVelocity);

	void UpdateSkinFromPlayerState();
	
	void ApplyFlagPhysics();

	// Replication Notifies
	UFUNCTION()
	void OnRep_InputEnabled();
	UFUNCTION()
	void OnRep_IsStun();
	UFUNCTION()
	void OnRep_IsInvincible();
	UFUNCTION()
	void OnRep_SkinColor();
	// ~Replication Notifies

	void ApplyEngineInputEnabled(const bool bEnable);

	FTimerHandle OnHitTimerHandle;
	FTimerHandle InvincibilityTimerHandle;

	/** Reasons for currently active input blocking. not replicated */
	uint8 InputBlockMask = 0;

	UPROPERTY(ReplicatedUsing = OnRep_InputEnabled)
	bool bInputEnabled = true;
	UPROPERTY(ReplicatedUsing = OnRep_IsInvincible)
	bool bIsInvincible = false;
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
	
	// TODO : 컴포지션으로 빼기
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
	
	// ~ Begin ACharacter Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void OnRep_Controller() override;
	// ~ End ACharacter Interface
	
public:
	
	//~ Begin Setter
	bool IsStun() const { return bIsStun; }
	bool IsRagdoll() const;
	bool IsInputBlocked() const { return InputBlockMask != 0; }
	UCharacterDataAsset* GetCharacterDataAsset() const { return CharacterData; }
	UTromboneRagdollComponent* GetRagdollComponent() const { return RagdollComponent; }
	//~ End Setter
};