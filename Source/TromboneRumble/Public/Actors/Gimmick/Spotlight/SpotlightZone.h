// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/ServerRPCInterface.h"
#include "SpotlightZone.generated.h"

class UAkCallbackInfo;
enum class EAkCallbackType : uint8;
class UAkAudioEvent;
class UAkComponent;
class UNiagaraSystem;
enum class ENoteResult : uint8;
class USpotLightComponent;
class USphereComponent;
class ADefaultTromboneCharacter;
class USpotlightGimmickConfig;

UENUM(BlueprintType)
enum class ESpotlightState : uint8
{
	None,       // 스폰 중
	Warning,    // 예고 (피버타임 아닐 시)
	Active,     // 활성 (점수 획득 가능)
	Fading      // 시간 초과로 사라짐
};

DECLARE_MULTICAST_DELEGATE(FOnSpotlightBonusEarned)

UCLASS()
class TROMBONERUMBLE_API ASpotlightZone : public AActor, public IServerRPCInterface
{
	GENERATED_BODY()
	
public:	
	ASpotlightZone();
	void InitializeZone(bool bIsFeverTime, const int32 InSpotlightBonusScore);
	virtual void HandleServerRPC(ACharacter* InstigatorCharacter) override;

	FOnSpotlightBonusEarned OnSpotlightBonusEarned;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void HandleTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp,	AActor* OtherActor,	UPrimitiveComponent* OtherComp,	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void HandleOnNoteDetected(ENoteResult NoteResult);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlaySpotlightTurnOnSFX();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlaySpotlightSuccessEffect(ADefaultTromboneCharacter* InPlayer);
	UFUNCTION(BlueprintImplementableEvent)
	void TurnOffLight();

	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> DefaultSceneRoot;

	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> TriggerVolume;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Components")
	TObjectPtr<USpotLightComponent> SpotLightComponent;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Components")
	TObjectPtr<UStaticMeshComponent> LightBeamMesh;

	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<UAkComponent> AkComponent;
private:
	/** Settings of the spotlight gimmick for this level. Never null. The phases of the zone come from here. */
	const USpotlightGimmickConfig& GetConfig() const;

	/**
	 * Start a phase of USpotlightGimmickConfig::GetZonePhases, or destroy the zone after the last one. Server only.
	 * The kind of the phase picks the state, so a skipped warning does not shift the others.
	 *
	 * @param PhaseIndex Index in the phases of this zone. An invalid index destroys the zone.
	 */
	void StartPhase(int32 PhaseIndex);

	/** Server only. Sets the state and plays what the new state plays. The phase timer is StartPhase's. */
	void SetState(ESpotlightState NewState);
	bool TryAwardBonus(ADefaultTromboneCharacter* InCharacter);
	
	UFUNCTION()
	void OnRep_CurrentState();

	
	
	
	UPROPERTY(ReplicatedUsing = OnRep_CurrentState)
	ESpotlightState CurrentState = ESpotlightState::None;
	
	UPROPERTY(Replicated)
	bool bIsBonusAwarded = false;
	
	

	/** 스포트라이트가 활성화(Active) 되었을 때 빛의 색상 */
	UPROPERTY(EditAnywhere, Category = "Spotlight|Config", meta = (DisplayName = "활성화 시 조명 색상"))
	FLinearColor SpotlightActiveColor = FLinearColor(1, 0.98f, 0.64f);
	
	FTimerHandle LifecycleTimerHandle;

	/** Was the zone made in fever time. The phases of the zone depend on it. Server only. */
	bool bSpawnedInFever = false;

	UPROPERTY(Replicated)
	int32 SpotlightBonusScore = 300;

	UPROPERTY(EditDefaultsOnly, Category = "VFX", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNiagaraSystem> SpotlightSuccessVFX;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAkAudioEvent> SpotLightTurnOnSFX;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAkAudioEvent> SpotlightSuccessSFX;

	UPROPERTY(Transient)
	bool bIsLocalPlayerOverlapping = false;
};