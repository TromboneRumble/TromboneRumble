// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Crown.generated.h"

// 1등 플레이어에게 부착될 왕관 액터
UCLASS()
class TROMBONERUMBLE_API ACrown : public AActor
{
	GENERATED_BODY()
	
public:
	ACrown();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	UFUNCTION()
	void HandleLeaderChanged(APlayerState* NewLeader, APlayerState* OldLeader);

	// 현재 붙어 있는 캐릭터
	UPROPERTY()
	TWeakObjectPtr<ACharacter> AttachedCharacter;

private:
	// InGameState에서 CurrentLeader를 못가져올 경우 대비해서 만들어진 타이머
	void TryAttachToInitialLeader();
	FTimerHandle InitialLeaderTimerHandle;

};
