// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/ServerRPCInterface.h"
#include "ResetCollider.generated.h"

class UBoxComponent;

/**
 * 공허로 떨어진 플레이어/아이템을 SpawnLocation으로 되돌리는 리셋 볼륨.
 *
 * - Overlap 경로: 박스에 닿은 Character / ItemBase를 텔레포트 (기존 BP_ResetCollider 로직)
 * - UI 경로: EscapePopup의 비상탈출 버튼 → ClientToServerRelayComponent → HandleServerRPC()
 *
 */
UCLASS()
class TROMBONERUMBLE_API AResetCollider : public AActor, public IServerRPCInterface
{
	GENERATED_BODY()

public:

	AResetCollider();

	// ~ Begin IServerRPCInterface
	virtual void HandleServerRPC(ACharacter* InstigatorCharacter) override;
	// ~ End IServerRPCInterface

	/** 레벨에 배치된 첫 번째 ResetCollider를 반환. 없으면 nullptr, 2개 이상이면 경고 로그 */
	static AResetCollider* FindInLevel(const UObject* WorldContextObject);


	void TeleportActorToSpawn(AActor* TargetActor);

protected:

	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBoxBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	// ~ Begin Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Config|Components")
	TObjectPtr<UBoxComponent> BoxComponent;
	// ~ End Components
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FVector SpawnLocation = FVector::ZeroVector;
};
