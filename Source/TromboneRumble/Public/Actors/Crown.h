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
	virtual void Tick(float DeltaSeconds) override;

protected:
	UFUNCTION()
	void HandleLeaderChanged(APlayerState* NewLeader, APlayerState* OldLeader);

	// 현재 붙어 있는 캐릭터
	UPROPERTY()
	TWeakObjectPtr<ACharacter> AttachedCharacter;

	// pelvis 위로 띄우는 기본 높이 (월드 수직 기준)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float HoverHeight = 130.f;

	// 위아래 흔들림 진폭
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float BobAmplitude = 10.f;

	// 위아래 흔들림 속도 (rad/s)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float BobSpeed = 2.f;

private:
	// 사인파 누적 시간
	float BobTime = 0.f;

	// InGameState에서 CurrentLeader를 못가져올 경우 대비해서 만들어진 타이머
	void TryAttachToInitialLeader();
	FTimerHandle InitialLeaderTimerHandle;

};
