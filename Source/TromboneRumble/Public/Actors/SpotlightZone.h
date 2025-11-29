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
	Awarded,    // 점수 획득 후 (폭죽 터짐)
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
private:
	void SetState(ESpotlightState NewState);
	bool TryAwardBonus(ADefaultTromboneCharacter* InCharacter);
	void StartLifecycleTimer(float InDuration, void (ASpotlightZone::*InTimerMethod)());

	void OnWarningFinished();
	void OnActiveFinished();
	void OnAwardedFinished();
	void OnFadingFinished();

	UFUNCTION()
	void OnSpotlightSuccessSFXFinished(EAkCallbackType InCallbackType, UAkCallbackInfo* InCallbackInfo);
	
	UFUNCTION()
	void OnRep_CurrentState();
	
	UPROPERTY(ReplicatedUsing = OnRep_CurrentState)
	ESpotlightState CurrentState = ESpotlightState::None;
	
	UPROPERTY(Replicated)
	bool bIsBonusAwarded = false;
	
	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> DefaultSceneRoot;

	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> TriggerVolume;

	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<USpotLightComponent> SpotLightComponent;
	
	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> LightBeamMesh;

	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<UAkComponent> AkComponent;

	UPROPERTY(EditAnywhere, Category = "Spotlight|Config")
	float WarningDuration = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Spotlight|Config")
	float ActiveDuration = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Spotlight|Config")
	float AwardedDuration = 2.0f;
	
	UPROPERTY(EditAnywhere, Category = "Spotlight|Config")
	float FadingDuration = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Spotlight|Config")
	FLinearColor SpotlightActiveColor = FLinearColor(1, 0.98f, 0.64f);

	UPROPERTY(EditAnywhere, Category = "Spotlight|Config")
	int32 SpotlightBonusScore = 300;

	FTimerHandle LifecycleTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "VFX", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNiagaraSystem> SpotlightSuccessVFX;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAkAudioEvent> SpotLightTurnOnSFX;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAkAudioEvent> SpotlightSuccessSFX;

	UPROPERTY(Transient)
	bool bIsLocalPlayerOverlapping = false;
};