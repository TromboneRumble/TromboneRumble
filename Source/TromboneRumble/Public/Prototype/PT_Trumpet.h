// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PT_Trumpet.generated.h"

UCLASS()
class TROMBONERUMBLE_API APT_Trumpet : public AActor
{
	GENERATED_BODY()
	
public:	
	APT_Trumpet();
	void OnGrab(bool isGrabbed, ACharacter* Parent);
	bool CanInteract() const { return !bIsGrabbed; }

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void PlaySound() const;
	void StopSound() const;

	UFUNCTION()
	void OnRep_Grabbed();
	
private:
	UPROPERTY(ReplicatedUsing = OnRep_Grabbed)
	uint8 bIsGrabbed = false;

	UPROPERTY(Replicated)
	TObjectPtr<ACharacter> LastOwningCharacter = nullptr;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UCapsuleComponent> CapsuleComponent;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USkeletalMeshComponent> TrumpetMesh = nullptr;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UAudioComponent> AudioComponent = nullptr;
	
};
