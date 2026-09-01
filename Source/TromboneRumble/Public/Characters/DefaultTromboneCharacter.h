// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Characters/TromboneCharacterBase.h"
#include "Components/ActorComponents/AttackComponent.h"
#include "Components/TimelineComponent.h"
#include "Data/CharacterDataAsset.h"
#include "Items/InstrumentBase.h"
#include "BlueprintFunctionLibraries/CameraFunctionLibrary.h"
#include "DefaultTromboneCharacter.generated.h"


class URageComponent;
class UMaterialInstanceDynamic;
class AWeaponBase;
struct FInputActionValue;
class ADefaultPlayerController;
class UTromboneVOIPTalker;

class UEquipmentComponent;
class UAkComponent;
class UAkAudioEvent;
class UClientToServerRelayComponent;
class UAttackComponent;
class USpringArmComponent;
class UCameraComponent;
class UInteractorComponent;
class UAbilitySystemComponent;
class URingHitBoxComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class UWidgetComponent;
class UCustomizationComponent;
class UTromboneRagdollComponent;
class UMaterialInterface;

class ARhythmActor;
class UCharacterDataAsset;
class UWeaponDataAsset;
class UCharacterAttributeSet;
class URhythmScoreAttributeSet;

class AItemBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkinColorChanged, const FLinearColor&, NewSkinColor);

UCLASS()
class TROMBONERUMBLE_API ADefaultTromboneCharacter : public ATromboneCharacterBase, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ADefaultTromboneCharacter();

	virtual void Jump() override;
	void Move(const FInputActionValue& Value);
	void TryInteract();
	void Attack();
	void StartSprint();
	void StopSprint();
	void Rhythm(bool bIsPressed);
	void Equip(AItemBase* WeaponToEquip);

	UFUNCTION()
	void Unequip();

	/** 마우스 휠 줌 단계 변경. WheelDelta: +1 = 줌인(레벨 감소), -1 = 줌아웃(레벨 증가) */
	void OnCameraZoom(float WheelDelta);

	// UVOIPTalker::OnTalkingBegin은 Listener에게만 적용되기 때문에, RPC를 통해 SpeakerIcon을 제어
	// True인 경우에는 해당 플레이어가 PushToTalk 모드를 사용해서 말을 하고 있음.
	UFUNCTION(Server, Reliable)
	void Server_SetSpeaking(bool bSpeaking);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetSpeaking(bool bSpeaking);

	/** 피부색 적용 (leader 메시 + follower 파츠). 변경을 OnSkinColorChanged 로 통지한다 (X-Ray 컴포넌트 등이 구독) */
	void ApplySkinColor(const FLinearColor InSkinColor);
	FLinearColor GetSkinColor() const { return SkinColor; }

	UPROPERTY(BlueprintAssignable)
	FOnSkinColorChanged OnSkinColorChanged;

	// 커스터마이징용 페이스 머티리얼 교체. nullptr 전달 시 원본 머티리얼로 복원
	void ApplyFaceMaterial(UMaterialInterface* Material);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCustomizationComponent> CustomizationComp;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void OnRep_Controller() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Config|Material")
	FName FaceExpressionParameterName = FName("ExpressionIndex");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Animation")
	TObjectPtr<UCurveVector> BounceCurve;

	// false면 SkinColor를 skin/face MID에 틴트하지 않고 머티리얼 기본색 사용 (PlayerState 없는 더미용)
	bool bApplySkinColorTint = true;

protected:
	// Components
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	/** 현재 줌 레벨 (1..3, 기본 2) */
	UPROPERTY(BlueprintReadWrite, Category = "Camera|Zoom")
	int32 CurrentZoomLevel = 2;

	/** 목표 Arm Length. BP Tick에서 UpdateTopDownCameraZoomEase에 전달 */
	UPROPERTY(BlueprintReadWrite, Category = "Camera|Zoom")
	float DesiredArmLength = 800.f;

	/** 목표 CameraBoom Rotation (Absolute). BP Tick에서 UpdateTopDownCameraZoomEase에 전달 */
	UPROPERTY(BlueprintReadWrite, Category = "Camera|Zoom")
	FRotator DesiredBoomRotation = FRotator(-30.f, 0.f, 0.f);

	/** 줌 보간 상태 (이상값 보존용) */
	UPROPERTY(BlueprintReadWrite, Category = "Camera|Zoom")
	FCameraZoomLerpState CameraZoomLerpState;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	TObjectPtr<UInteractorComponent> InteractorComponent;

	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<UAttackComponent> AttackComponent;

	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<UEquipmentComponent> EquipmentComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<URageComponent> RageComponent;

	UPROPERTY()
	TObjectPtr<UCharacterAttributeSet> CharacterAttributes;

	UPROPERTY()
	TObjectPtr<URhythmScoreAttributeSet> RhythmScoreAttributes;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	TObjectPtr<UClientToServerRelayComponent> ServerRelayComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<URingHitBoxComponent> RingHitBoxComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components|UI")
	TObjectPtr<USceneComponent> ComboWidgetAnchorComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Voice")
	TObjectPtr<UTromboneVOIPTalker> VOIPTalker;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Voice")
	TObjectPtr<UWidgetComponent> SpeakerIndicatorComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components|UI")
	TObjectPtr<UWidgetComponent> ComboWidgetComponent;
	// ~Components

	UPROPERTY(EditDefaultsOnly, Category = "DefaultWeapon")
	TSubclassOf<AWeaponBase> DefaultWeaponClass = nullptr;

	// TODO : 빙의를 하지 않는 더미 캐릭터의 경우 PossessedBy가 호출되지 않아 null임. 수정 필요
	// @see ATutorialDummy
	UPROPERTY(Transient)
	TObjectPtr<AWeaponBase> DefaultWeaponInstance = nullptr;

	UPROPERTY(Transient)
	TWeakObjectPtr<ADefaultPlayerController> CachedCharacterController;

	UPROPERTY(Transient)
	TWeakObjectPtr<ARhythmActor> CachedRhythmActor;

protected:
	//~ Begin ATromboneCharacterBase Interface
	/** Players move by input, so a block turns the input off. */
	virtual void OnBlockedStateChanged(bool bBlocked) override;
	//~ End ATromboneCharacterBase Interface

private:
	void UpdateMaxWalkSpeed();

	// Server RPCs
	UFUNCTION(Server, Reliable)
	void Server_SetIsSprinting(const bool bNewIsSprinting);
	UFUNCTION(Server, Reliable)
	void Server_InteractItem(AItemBase* InteractedItem);
	// ~Server RPCs

	// Delegate Callback Handlers
	UFUNCTION()
	void HandleInteractableAvailableChanged(bool bAvailable);
	UFUNCTION()
	void HandleInteractSuccess(AActor* InteractedActor);
	UFUNCTION()
	void HandleOnEquipmentChanged(EEquipmentSlotType Slot, AItemBase* NewItem, AItemBase* OldItem);
	// ~Delegate Callback Handlers

	ARhythmActor* GetCachedRhythmActor();
	void SpawnAndEquipDefaultWeapon();
	void SpawnAndEquipPreviouslyEquippedWeapon();

	UPROPERTY(Replicated)
	uint8 bIsSprinting : 1 = 0;

	// Voice Interaction
	void TryRegisterVOIPTalker();

	FTimerHandle RetryVOIPRegistrationHandle;
	// ~Voice Interaction

	// ~ Begin 상태 연출 (상태 전이는 베이스가 소유, 여기서는 델리게이트 구독으로 연출만 처리)
	UFUNCTION()
	void HandleStunStateChanged(bool bIsStunned);
	UFUNCTION()
	void HandleRagdollStartedVisuals();
	UFUNCTION()
	void HandleRagdollEndedVisuals();
	UFUNCTION()
	void HandleRagdollPhysicsEnabled();

	void ApplyFlagPhysics();
	// ~ End 상태 연출

	// ~ Begin 외형 / 표정
	// TODO : 컴포지션으로 빼기
	void SetupCharacterData() const;
	void UpdateSkinFromPlayerState();

	UFUNCTION()
	void OnRep_SkinColor();

	UPROPERTY(ReplicatedUsing = OnRep_SkinColor)
	FLinearColor SkinColor = FLinearColor::Black;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> SkinMID;
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> FaceMID;
	// BeginPlay에서 FaceMID 생성 직전 원본 머티리얼 캐싱 (커스터마이징 복원용)
	UPROPERTY()
	TObjectPtr<UMaterialInterface> OriginalFaceMaterial;

	void PlayFaceSequence(ECharacterFaceState TargetState);
	void InternalPlayFaceSequence(const FCharacterFaceAnimationSequence* InSequence);
	void ExecuteFaceStep();
	void UpdateFaceExpression(ECharacterFaceType NewType);

	int32 CurrentSequenceStep = 0;
	FCharacterFaceAnimationSequence CurrentActiveSequence;
	FTimerHandle FaceSequenceTimerHandle;
	// ~ End 외형 / 표정

	// ~ Bounce Character
	FTimeline BounceTimeline;
	void BoundBounceTimeline();
	UFUNCTION()
	void HandleBounceProgress(FVector Value);
	// ~ End Bounce Character

public:
	// ~ Begin Getters / Setters
	FORCEINLINE UClientToServerRelayComponent* GetClientToServerRelayComponent() const { return ServerRelayComponent; }
	FORCEINLINE UAkComponent* GetAkComponent() { return AkSoundComponent; }
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
	FORCEINLINE TObjectPtr<AWeaponBase> GetCurrentWeapon() const { return AttackComponent ? AttackComponent->GetCurrentWeapon() : nullptr; }
	FORCEINLINE UWidgetComponent* GetComboWidgetComponent() { return ComboWidgetComponent; }
	FORCEINLINE UEquipmentComponent* GetEquipmentComponent() const { return EquipmentComponent; }
	FORCEINLINE bool IsSprinting() const { return bIsSprinting; }
	EInstrumentType GetCurrentEquippedInstrumentType() const;

	// 얼음 위 걷기 애니메이션 배속 (AnimInstance 가 로코모션 Play Rate 로 사용). 속성 없으면 1.0
	float GetLocomotionPlayRate() const;
	// ~ End Getters / Setters
};
