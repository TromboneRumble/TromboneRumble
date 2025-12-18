// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GarbageBase.generated.h"

class UStaticMeshComponent;
class UNiagaraComponent;

UCLASS(Abstract)
class TROMBONERUMBLE_API AGarbageBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AGarbageBase();

	UFUNCTION(BlueprintCallable, Category = "Garbage")
	void InitThrow_Server(const FVector& InStart, const FVector& InTarget);

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Garbage|Components")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Garbage|Components")
	TObjectPtr<UNiagaraComponent> TrailComp;
	// ~Components

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Garbage|Throw")
	float MinExtraApexHeight = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Garbage|Throw")
	float MaxExtraApexHeight = 450.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Garbage|Throw")
	float MinSpinDegPerSec = 180.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Garbage|Throw")
	float MaxSpinDegPerSec = 900.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Garbage|Lifetime")
	float DestroyDelayAfterLand = 2.0f;

	// 땅 또는 플레이어와 처음 충돌한 시점부터 N초 후 삭제
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Garbage|Lifetime")
	float DestroyDelayAfterImpact = 2.0f;

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

	UFUNCTION()
	void OnRep_ImpactStarted();

protected:
	UFUNCTION()
	void HandleMeshHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

	FVector ComputeBallisticInitialVelocity(const FVector& InStart, const FVector& InTarget, float ExtraApexHeight) const;
	void StartDestroyTimer_Server();

private:
	bool bDestroyTimerStarted = false;

};
