// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Present.generated.h"

class UAkAudioEvent;
class UAkComponent;
class USphereComponent;

UCLASS()
class TROMBONERUMBLE_API APresent : public AActor
{
	GENERATED_BODY()

public:
	APresent();

	UPROPERTY(EditAnywhere, Category = "Present|Config", meta = (DisplayName = "획득 점수"))
	int32 BonusScore = 300;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UStaticMeshComponent> GiftMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BalloonMesh;

	/** 진자 회전 중심 (풍선과 선물을 연결하는 축) */
	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> PendulumPivot;

	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> OverlapSphere;

	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<UAkComponent> AkComponent;

	/** 진자 최대 흔들림 각도 (도) */
	UPROPERTY(EditAnywhere, Category = "Present|Config", meta = (DisplayName = "진자 최대 각도 (도)"))
	float MaxPendulumAngleDeg = 30.f;

	/** 진자 길이 - 풍선과 선물 사이 거리 (cm) */
	UPROPERTY(VisibleInstanceOnly, Category = "Present|Config", meta = (DisplayName = "계산된 진자 길이 (cm)"))
	float PendulumLength;

	/** 흔들림 주기 (Hz) */
	UPROPERTY(EditAnywhere, Category = "Present|Config", meta = (DisplayName = "흔들림 주기 (Hz)"))
	float SwayFrequency = 1.2f;

	/** 낙하 가속도 (cm/s²) */
	UPROPERTY(EditAnywhere, Category = "Present|Config", meta = (DisplayName = "낙하 가속도"))
	float GravityAccel = 300.f;

	/** 초기 낙하 속도 (cm/s) */
	UPROPERTY(EditAnywhere, Category = "Present|Config", meta = (DisplayName = "초기 낙하 속도"))
	float InitialFallSpeed = 50.f;

	/** 바닥 충돌 후 자동 소멸 시간 (초) */
	UPROPERTY(EditAnywhere, Category = "Present|Config", meta = (DisplayName = "착지 후 소멸 시간 (초)"))
	float LifetimeAfterLanding = 5.f;

	/** 플레이어 충돌 시 재생되는 단발성 사운드 */
	UPROPERTY(EditDefaultsOnly, Category = "Present|Audio")
	TObjectPtr<UAkAudioEvent> PresentHitSoundEvent;

private:
	UFUNCTION()
	void HandleOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(Server, Reliable)
	void Server_OnPlayerTouched();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_HideBalloon();

	UPROPERTY(ReplicatedUsing = OnRep_HasLanded)
	bool bHasLanded = false;
	
	UPROPERTY(Replicated)
	FVector LandedLocation;

	UPROPERTY(Replicated)
	FRotator LandedRotation;

	UPROPERTY(Replicated)
	FVector AnchorSpawnLocation;

	UPROPERTY(Replicated)
	FVector ReplicatedSwayAxis;

	UPROPERTY(Replicated)
	float ReplicatedSwayPhaseOffset;
	
	UFUNCTION()
	void OnRep_HasLanded();

	bool bBonusAwarded = false;
	float SwayTime = 0.f;
};
