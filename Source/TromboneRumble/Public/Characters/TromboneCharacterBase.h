// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CharacterDataAsset.h"
#include "GameFramework/Character.h"
#include "Interfaces/CombatReceiver.h"
#include "TromboneCharacterBase.generated.h"

class UTromboneRagdollComponent;
class UAkComponent;
class UAkAudioEvent;
class UNiagaraComponent;
class UPhysicalAnimationComponent;
class UCharacterDataAsset;

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

/** ATromboneCharacterBase
 *
 * "맞을 수 있는 캐릭터"의 베이스. 게임 컨셉상 모든 캐릭터(플레이어/NPC)는 물리 기반이며 래그돌될 수 있다.
 * - ICombatReceiver 구현: 넉백 계산 + 리액션 디스패치
 * - 상태 소유: 스턴/무적/래그돌 + 전이 로직 + 상태 수치(CharacterData)
 * - 입력 잠금(비트 마스크) 및 서버 입력 잠금 복제
 * - 공용 상태 연출(스턴 이펙트/사운드, 래그돌 야유)은 베이스가 재생한다. 특정 메시/머티리얼 구조에
 *   결합된 연출(표정, Bounce 등)과 외형(커마/스킨)은 소유하지 않는다 — 상태 전이는 델리게이트
 *   (OnStunStateChanged, RagdollComponent 의 OnRagdollStarted 등)로 통지되고,
 *   파생 클래스가 구독하여 자기 연출을 얹는다. (플레이어 = ADefaultTromboneCharacter)
 */
UCLASS()
class TROMBONERUMBLE_API ATromboneCharacterBase : public ACharacter, public ICombatReceiver
{
	GENERATED_BODY()

public:
	ATromboneCharacterBase();

	// ~ Begin ICombatReceiver Interfaces
	virtual bool OnHitReceived_Implementation(const FHitData& HitData) override;
	// ~ End ICombatReceiver Interfaces

	/** Add a reason for the input lock */
	void AddInputBlock(EInputBlockReason Reason);

	/** Remove a reason for the input lock */
	void RemoveInputBlock(EInputBlockReason Reason);

	/** ServerLock 비트를 리플리케이트해 각 머신의 비트 마스크에 적용. Server Only. */
	void Server_SetInputEnabled(const bool bEnable);

	/** 상태 전이 통지. 파생(연출)과 외부(RingHitBox 등)가 구독한다 */
	FOnStunStateChanged OnStunStateChanged;
	FInvincibleSignature OnInvincibleDelegate;
	FInvincibleSignature EndInvincibleDelegate;

	/** 피격을 수용할 수 있는 상태인지. 파생에서 추가 조건(퇴장 중 판정 비활성 등)을 얹을 수 있다 */
	virtual bool CanReceiveHit() const { return !(bIsInvincible || bIsStun || IsRagdoll()); }

	/** Called when the get-up montage finishes. The character can move again from here. */
	virtual void HandleGetUpFinished() {}

	/** Called when the character falls into the beer. Server only. */
	virtual void HandleDrowningStarted() {}

	/** Called when the beer drains and the character is free. Server only. */
	virtual void HandleDrowningEnded() {}

	/** Plays the falling scream on every machine. Server only. */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayFallScream();

	/** Watches for the first hard landing and plays the landing sound. Only the lobby turns this on. Server only. */
	void SetLandingSoundEnabled(bool bEnable);

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Data")
	TObjectPtr<UCharacterDataAsset> CharacterData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Components")
	TObjectPtr<UTromboneRagdollComponent> RagdollComponent;

	UPROPERTY(EditAnywhere, Category = "Config|Components|Sound")
	TObjectPtr<UAkComponent> AkSoundComponent;

	// 공용 상태 연출 자산: 모든 캐릭터가 스턴/래그돌 되므로 베이스가 재생. 파생 BP에서 교체/제거 가능
	UPROPERTY(EditDefaultsOnly, Category = "Config|Components|Niagara")
	TObjectPtr<UNiagaraComponent> StunNiagaraComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Config|Sound")
	TObjectPtr<UAkAudioEvent> StunNiagaraSound;

	UPROPERTY(EditDefaultsOnly, Category = "Config|Sound")
	TObjectPtr<UAkAudioEvent> RagdollBooSound;

	UPROPERTY(EditDefaultsOnly, Category = "Config|Sound", meta = (DisplayName = "로비 낙하 연출 - 낙하 비명 사운드"))
	TObjectPtr<UAkAudioEvent> FallScreamSound;

	UPROPERTY(EditDefaultsOnly, Category = "Config|Sound", meta = (DisplayName = "로비 낙하 연출 - 착지 비명 사운드"))
	TObjectPtr<UAkAudioEvent> LandPainSound;

	UPROPERTY(EditDefaultsOnly, Category = "Config|Sound", meta = (DisplayName = "착지 판정 충격량", ClampMin = "0.0"))
	float LandingImpulseThreshold = 20000.f;

	UFUNCTION()
	void HandleRagdollLandingHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayLandPain();

	/** 물리 애니메이션 (플레이어: 깃발, NPC: 상체 흐느적거림 등 파생 공용) */
	UPROPERTY()
	TObjectPtr<UPhysicalAnimationComponent> PhysicalAnimationComp;

private:

	void OnStun();
	void EndStun();

	void ApplyStun();
	void UnapplyStun();

	/** 래그돌 시작/종료 시의 상태 처리(스턴 해제, 입력 잠금, 무적 타이머) + 공용 연출(야유 사운드).
	 *  파생 전용 연출(표정 등)은 파생이 같은 래그돌 델리게이트를 구독해 별도 핸들러로 처리한다 */
	UFUNCTION()
	void HandleRagdollStarted();
	UFUNCTION()
	void HandleRagdollEnded();

	/** FHitData 를 최종 넉백 속도로 계산한다. 폭발이면 방사형, 아니면 수평 힘 + 수직 힘 조합 */
	FVector CalculateKnockbackVelocity(const FHitData& HitData) const;

	/** @return 넉백으로 쓰러질 때 몸에 걸어줄 각속도. 밀려나는 방향으로 굴러가도록 진행 방향을 축으로 잡는다 */
	FVector CalculateKnockbackSpin(const FVector& KnockbackVelocity) const;

	UFUNCTION(Client, Reliable)
	void Client_ApplyKnockback(FVector KnockbackVelocity);

	// Replication Notifies
	UFUNCTION()
	void OnRep_InputEnabled();
	UFUNCTION()
	void OnRep_IsStun();
	UFUNCTION()
	void OnRep_IsInvincible();
	// ~Replication Notifies

	void ApplyEngineInputEnabled(const bool bEnable);

	FTimerHandle OnHitTimerHandle;
	FTimerHandle InvincibilityTimerHandle;

	/** Reasons for currently active input blocking. not replicated */
	uint8 InputBlockMask = 0;

	int32 StunNiagaraPlayingID = 0;

	UPROPERTY(ReplicatedUsing = OnRep_InputEnabled)
	bool bInputEnabled = true;
	UPROPERTY(ReplicatedUsing = OnRep_IsInvincible)
	bool bIsInvincible = false;
	UPROPERTY(ReplicatedUsing = OnRep_IsStun)
	bool bIsStun = false;

public:

	// ~ Begin ACharacter Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// ~ End ACharacter Interface

public:

	// ~ Begin Getters
	bool IsStun() const { return bIsStun; }
	bool IsInvincible() const { return bIsInvincible; }
	bool IsRagdoll() const;

	/** @return World location of the pelvis bone. */
	FVector GetPelvisLocation() const;
	
	bool IsInputBlocked() const { return InputBlockMask != 0; }
	UAkComponent* GetAkComponent() const { return AkSoundComponent; }
	UCharacterDataAsset* GetCharacterDataAsset() const { return CharacterData; }
	UTromboneRagdollComponent* GetRagdollComponent() const { return RagdollComponent; }
	// ~ End Getters
};
