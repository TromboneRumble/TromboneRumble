// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Components/ActorComponents/DrunkardStateComponent.h"
#include "DrunkardAIController.generated.h"

class UBehaviorTree;

/** ADrunkardAIController
 *
 * 취객 NPC의 BT 실행자. 게임 규칙은 UDrunkardStateComponent가 소유하고,
 * 이 컨트롤러는 상태/타겟 변경 델리게이트를 구독해 블랙보드에 반영만 한다.
 */
UCLASS()
class TROMBONERUMBLE_API ADrunkardAIController : public AAIController
{
	GENERATED_BODY()

public:
	// Blackboard 키 이름 (BT 에셋의 키와 일치해야 한다)
	static const FName BBKeyTargetPlayer;	// Object: 현재 추격 대상
	static const FName BBKeyMoveGoal;		// Vector: 위빙 보정이 적용된 이동 목표
	static const FName BBKeyState;			// Enum(EDrunkardState)
	static const FName BBKeyExitDoor;		// Object: 퇴장할 문 (종료 시점 최근접)

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Config|AI", meta = (DisplayName = "행동 트리"))
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

private:
	UFUNCTION()
	void HandleStateChanged(EDrunkardState NewState);

	UFUNCTION()
	void HandleTargetChanged(ADefaultTromboneCharacter* NewTarget);

	UPROPERTY(Transient)
	TObjectPtr<UDrunkardStateComponent> StateComponent;

protected:
	//~ Begin AController Interface
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	//~ End AController Interface
};
