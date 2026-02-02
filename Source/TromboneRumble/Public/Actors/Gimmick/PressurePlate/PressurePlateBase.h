// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Gimmick/GimmickBase.h"
#include "PressurePlateBase.generated.h"

class UTimelineComponent;
class UBoxComponent;

UCLASS(Abstract)
class TROMBONERUMBLE_API APressurePlateBase : public AGimmickBase
{
	GENERATED_BODY()
	
public:	
	APressurePlateBase();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|PressurePlate")
	TArray<TSubclassOf<AActor>> SpawningActorClasses;;

	// Components
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Platform;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Frame;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> PressTargetLocation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Timeline")
	TObjectPtr<UTimelineComponent> PressureTimeline;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Timeline")
	TObjectPtr<UCurveFloat> PressureCurve;

	UPROPERTY(VisibleAnywhere, Category = "Components|Timeline")
	TObjectPtr<UTimelineComponent> SpawnRiseTimeline;

	UPROPERTY(EditAnywhere, Category = "Components|Timeline")
	TObjectPtr<UCurveFloat> SpawnRiseCurve;
	// ~Components
private:


	FVector InitialLocation;

	int32 OverlappingCount = 0;

	UPROPERTY(ReplicatedUsing = OnRep_IsPressed)
	bool bIsPressed;

	UFUNCTION()
	void OnRep_IsPressed();

	UFUNCTION()
	void OnTimelineFinished();

	UFUNCTION(Server, Reliable)
	virtual void Server_OnPlateActivated();

	UFUNCTION(BlueprintCallable)
	void UpdatePlateLocation(const float InAlpha);

	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
