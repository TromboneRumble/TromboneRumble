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

UENUM(BlueprintType)
enum class ESpotlightState : uint8
{
	None,       // 스폰 중
	Warning,    // 예고 (피버타임 아닐 시)
	Active,     // 활성 (점수 획득 가능)
	Fading      // 시간 초과로 사라짐
};

UCLASS()
class TROMBONERUMBLE_API ASpotlightZone : public AActor, public IServerRPCInterface
{
	GENERATED_BODY()
	
public:	
	ASpotlightZone();
	void InitializeZone(bool bIsFeverTime, const int32 InSpotlightBonusScore);
	virtual void HandleServerRPC(ACharacter* InstigatorCharacter) override;
	

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
	void SetState(ESpotlightState NewState);
	bool TryAwardBonus(ADefaultTromboneCharacter* InCharacter);
	void StartLifecycleTimer(float InDuration, void (ASpotlightZone::*InTimerMethod)());

	void OnWarningFinished();
	void OnActiveFinished();
	void OnFadingFinished();
	
	UFUNCTION()
	void OnRep_CurrentState();

	
	
	
	UPROPERTY(ReplicatedUsing = OnRep_CurrentState)
	ESpotlightState CurrentState = ESpotlightState::None;
	
	UPROPERTY(Replicated)
	bool bIsBonusAwarded = false;
	
	

	/** 스포트라이트가 생성된 후 본격적으로 활성화되기 전까지의 대기 시간 (초) */
	UPROPERTY(EditAnywhere, Category = "Spotlight|Config", meta = (DisplayName = "경고 상태 지속 시간"))
	float WarningDuration = 1.5f;

	/** 플레이어가 점수를 획득할 수 있는 실질적인 유지 시간 (초) */
	UPROPERTY(EditAnywhere, Category = "Spotlight|Config", meta = (DisplayName = "활성 상태 지속 시간"))
	float ActiveDuration = 3.0f;
	
	/** 스포트라이트가 시간 초과 등으로 인해 서서히 사라지는 단계의 시간 (초) */
	UPROPERTY(EditAnywhere, Category = "Spotlight|Config", meta = (DisplayName = "소멸 단계 지속 시간"))
	float FadingDuration = 2.0f;

	/** 스포트라이트가 활성화(Active) 되었을 때 빛의 색상 */
	UPROPERTY(EditAnywhere, Category = "Spotlight|Config", meta = (DisplayName = "활성화 시 조명 색상"))
	FLinearColor SpotlightActiveColor = FLinearColor(1, 0.98f, 0.64f);
	
	FTimerHandle LifecycleTimerHandle;

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