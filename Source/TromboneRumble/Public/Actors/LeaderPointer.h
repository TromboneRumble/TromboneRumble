// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LeaderPointer.generated.h"

class UFloatingRotatingComponent;

UCLASS(Abstract)
class TROMBONERUMBLE_API ALeaderPointer : public AActor
{
	GENERATED_BODY()
	
public:	
	ALeaderPointer();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleLeaderChanged(APlayerState* NewLeader, APlayerState* OldLeader);

	UPROPERTY(EditDefaultsOnly)
	FVector LocalPlayerDistanceOffset = FVector(0.f,0.f, 185.f);

	UPROPERTY(EditDefaultsOnly)
	FVector OtherPlayerDistanceOffset = FVector(0.f, 0.f, 140.f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UFloatingRotatingComponent* MovementComponent;

	UPROPERTY()
	TWeakObjectPtr<ACharacter> AttachedCharacter;

private:
	// InGameState에서 CurrentLeader를 못가져올 경우 대비해서 만들어진 타이머
	void TryAttachToInitialLeader();
	FTimerHandle InitialLeaderTimerHandle;
};
