// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Interactable.h"
#include "Components/TimelineComponent.h"
#include "Utilities/Defines.h"
#include "PressurePlate_Explosive.generated.h"

enum class EHitInstigatorType : uint8;
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

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> ExplosionSphere;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UTimelineComponent> EmissiveTimeline;

	UPROPERTY(EditAnywhere, Category = "Config|Gimmick")
	TObjectPtr<UCurveFloat> EmissiveCurve;

	UPROPERTY(EditAnywhere, Category = "Config|Gimmick")
	float PreExplosionTime = 2.f;

	UPROPERTY(EditAnywhere, Category = "Config|Gimmick", meta = (ClampMin = "0.0"))
	float ExplosionStrength = 1500.f;

	UPROPERTY(EditAnywhere, Category = "Config|Gimmick", meta = (ClampMin = "0.0"))
	float UpwardImpulseBoost = 800.f;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "Config|Gimmick")
	EHitInstigatorType HitInstigatorType = EHitInstigatorType::PressurePlate;

private:
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial;
};
