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
	// ~Components

	UPROPERTY(EditAnywhere, Category = "Garbage|Throw")
	float MinExtraApexHeight = 150.f;

	UPROPERTY(EditAnywhere, Category = "Garbage|Throw")
	float MaxExtraApexHeight = 450.f;

	UPROPERTY(EditAnywhere, Category = "Garbage|Throw")
	float MinSpinDegPerSec = 180.f;

	UPROPERTY(EditAnywhere, Category = "Garbage|Throw")
	float MaxSpinDegPerSec = 900.f;

	UPROPERTY(EditAnywhere, Category = "Garbage|Lifetime")
	float DestroyDelayAfterLand = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Garbage|Lifetime")
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
	void StartDestroyTimer_Server(float Delay);

private:
	bool bDestroyTimerStarted = false;
};