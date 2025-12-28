// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utilities/Defines.h"
#include "GarbageBase.generated.h"

class UAkAudioEvent;
class UAkComponent;
class UStaticMeshComponent;
class UNiagaraComponent;

UCLASS(Abstract)
class TROMBONERUMBLE_API AGarbageBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AGarbageBase();

	UFUNCTION(Category = "Garbage")
	void InitThrow_Server(const FVector& InStart, const FVector& InTarget);

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Components
	UPROPERTY(VisibleAnywhere, Category = "Garbage|Components")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Garbage|Components")
	TObjectPtr<UNiagaraComponent> TrailComp;

	UPROPERTY(VisibleAnywhere, Category = "Garbage|Components")
	TObjectPtr<UAkComponent> AkComponent;
	// ~Components

	UPROPERTY(EditDefaultsOnly, Category = "Garbage|Sound")
	TObjectPtr<UAkAudioEvent> HitSoundEvent = nullptr;

	/** 투척 시 최소 추가 높이 (cm) 
	 * 시작점과 목표점 중 높은 곳을 기준으로, 최소한 이 값만큼은 더 위로 솟구쳤다가 떨어집니다. */
	UPROPERTY(EditAnywhere, Category = "Garbage|Config|Throw", meta = (DisplayName = "최소 추가 정점 높이"))
	float MinExtraApexHeight = 150.f;

	/** 투척 시 최대 추가 높이 (cm) 
	 * 시작점과 목표점 중 높은 곳을 기준으로, 최대한 이 값까지 위로 솟구칠 수 있습니다. */
	UPROPERTY(EditAnywhere, Category = "Garbage|Config|Throw", meta = (DisplayName = "최대 추가 정점 높이"))
	float MaxExtraApexHeight = 450.f;

	/** 초당 최소 회전 속도 (도/sec) 
	 * 날아가는 동안 무작위 축으로 회전할 최소 속도입니다. */
	UPROPERTY(EditAnywhere, Category = "Garbage|Config|Throw", meta = (DisplayName = "최소 회전 속도"))
	float MinSpinDegPerSec = 180.f;

	/** 초당 최대 회전 속도 (도/sec) 
	 * 날아가는 동안 무작위 축으로 회전할 최대 속도입니다. */
	UPROPERTY(EditAnywhere, Category = "Garbage|Config|Throw", meta = (DisplayName = "최대 회전 속도"))
	float MaxSpinDegPerSec = 900.f;

	/** 착지 후 제거 대기 시간 (초) 
	 * 바닥(WorldStatic/Dynamic)에 닿은 후 액터가 파괴될 때까지의 시간입니다. */
	UPROPERTY(EditAnywhere, Category = "Garbage|Config|Lifetime", meta = (DisplayName = "착지 후 제거 지연"))
	float DestroyDelayAfterLand = 5.0f;

	/** 충돌 후 제거 대기 시간 (초) 
	 * 캐릭터 등과 충돌한 후 액터가 파괴될 때까지의 시간입니다. */
	UPROPERTY(EditAnywhere, Category = "Garbage|Config|Lifetime", meta = (DisplayName = "충돌 후 제거 지연"))
	float DestroyDelayAfterImpact = 2.0f;
	
	/** 피해 유형 
	 * 이 투척물이 캐릭터와 충돌했을 때 적용할 피해 유형입니다. */
	UPROPERTY(EditAnywhere, Category = "Garbage|Config|HitType", meta = (DisplayName = "피해 유형"))
	EHitReactionType HitReactionType = EHitReactionType::None;

protected:
	// Replication
	UPROPERTY(Replicated)
	FVector_NetQuantize10 StartLoc;

	UPROPERTY(Replicated)
	FVector_NetQuantize10 TargetLoc;

	UPROPERTY(Replicated)
	float ChosenExtraApexHeight = 0.f;

	UPROPERTY(Replicated)
	FRotator SpinRateDegPerSec = FRotator::ZeroRotator;

	UPROPERTY(ReplicatedUsing = OnRep_ImpactStarted)
	bool bImpactStarted = false;

	UPROPERTY(ReplicatedUsing = OnRep_HitPawn)
	bool bHitPawn = false;

	UFUNCTION()
	void OnRep_ImpactStarted();

	UFUNCTION()
	void OnRep_HitPawn();

protected:
	UFUNCTION()
	void HandleMeshHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

private:
	FVector ComputeBallisticInitialVelocity(const FVector& InStart, const FVector& InTarget, float ExtraApexHeight) const;
	void StartDestroyTimer_Server(float Delay);

	bool bDestroyTimerStarted = false;
};