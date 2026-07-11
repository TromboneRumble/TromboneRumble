// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Characters/TromboneCharacterBase.h"
#include "Components/ActorComponents/AttackComponent.h"
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
class UClientToServerRelayComponent;
class UAttackComponent;
class USpringArmComponent;
class UCameraComponent;
class UInteractorComponent;
class UAbilitySystemComponent;
class URingHitBoxComponent;
class UNiagaraSystem;
class UWidgetComponent;

class ARhythmActor;
class UCharacterDataAsset;
class UWeaponDataAsset;
class UCharacterAttributeSet;
class URhythmScoreAttributeSet;

class AItemBase;

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

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	// 피부색 적용 시 X-Ray 실루엣 MID 색상도 함께 갱신 (로컬 플레이어 한정)
	virtual void ApplySkinColor(const FLinearColor InSkinColor) const override;

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
	
	// 가려졌을 때 X-Ray 실루엣 표시용 PostProcess 머티리얼 (로컬 플레이어 카메라에만 블렌드)
	UPROPERTY(EditDefaultsOnly, Category = "Config|Camera|Occlusion")
	TObjectPtr<UMaterialInterface> OcclusionOverlayMaterial;

	// OcclusionOverlayMaterial의 동적 인스턴스. SilhouetteColor를 로컬 플레이어 피부색으로 주입.
	// 로컬 플레이어 카메라에만 존재(원격 캐릭터에서는 null)
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> OcclusionOverlayMID;

	// X-Ray 실루엣 색상으로 사용할 PostProcess 머티리얼의 VectorParameter 이름
	static const FName SilhouetteColorParamName;

private:
	void UpdateMaxWalkSpeed();
	
	// Server RPCs
	UFUNCTION(Server, Reliable)
	void Server_SetIsSprinting(const bool bNewIsSprinting);
	UFUNCTION(Server, Reliable)
	void Server_InteractItem(AItemBase* InteractedItem);
	// ~Server RPCs
	
	//카메라→캐릭터 트레이스로 XRayBlocker 태그 감지
	FTimerHandle XRayTraceTimerHandle;
	UFUNCTION()
	void CheckXRayOcclusion();
	
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