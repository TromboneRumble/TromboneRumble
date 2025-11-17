// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpotlightZone.generated.h"

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
class TROMBONERUMBLE_API ASpotlightZone : public AActor
{
	GENERATED_BODY()
	
public:	
	ASpotlightZone();
	void InitializeZone(bool bIsFeverTime);
	bool AttemptToAwardBonus(ADefaultTromboneCharacter* Player);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	void SetState(ESpotlightState NewState);
	void StartLifecycleTimer(float InDuration, void (ASpotlightZone::*InTimerMethod)());

	void OnWarningFinished();
	void OnActiveFinished();
	void OnAwardedFinished();
	void OnFadingFinished();
	
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
	TObjectPtr<UDecalComponent> DecalComponent;

	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> LightBeamMesh;

	UPROPERTY(EditAnywhere, Category = "Spotlight|Config")
	float WarningDuration = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Spotlight|Config")
	float ActiveDuration = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Spotlight|Config")
	float AwardedDuration = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Spotlight|Visuals")
	TObjectPtr<UMaterialInterface> WarningMaterial;
    
	UPROPERTY(EditAnywhere, Category = "Spotlight|Visuals")
	TObjectPtr<UMaterialInterface> ActiveMaterial;

	FTimerHandle LifecycleTimerHandle;
};