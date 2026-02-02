// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Interactable.h"
#include "Components/TimelineComponent.h"
#include "PressurePlate_Explosive.generated.h"

class USphereComponent;

UCLASS()
class TROMBONERUMBLE_API APressurePlate_Explosive : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	APressurePlate_Explosive();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	// ~ Begin IInteractable Interfaces
	virtual bool CanInteract_Implementation(AActor* InstigatorActor) const override;
	virtual void Interact_Implementation(AActor* InstigatorActor) override;
	// ~ End IInteractable Interfaces

	UFUNCTION()
	void UpdateEmissiveEffect(float Value);

	UFUNCTION()
	void StartExplosionExpansion();

	UFUNCTION()
	void UpdateExplosionRadius(float Value);

	UFUNCTION()
	void OnExpansionFinished();

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> ExplosionSphere;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UTimelineComponent> EmissiveTimeline;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UTimelineComponent> ExpansionTimeline;

	UPROPERTY(EditAnywhere, Category = "Config|Gimmick")
	TObjectPtr<UCurveFloat> EmissiveCurve;

	UPROPERTY(EditAnywhere, Category = "Config|Gimmick")
	TObjectPtr<UCurveFloat> ExpansionCurve;

	UPROPERTY(EditAnywhere, Category = "Config|Gimmick")
	float PreExplosionTime = 2.f;

private:
	UPROPERTY(EditAnywhere, Category = "Config|Gimmick")
	float MaxRadius = 600.f;

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial;
};
