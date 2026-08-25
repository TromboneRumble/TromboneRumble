// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Utilities/Defines.h"
#include "CombatReceiver.generated.h"

USTRUCT(BlueprintType)
struct FHitData
{
	GENERATED_BODY()

	/** 넉백 방향. 수신 측에서 수평 성분만 사용한다 (수직은 KnockbackUpForce 로 별도 지정) */
	UPROPERTY()
	FVector HitDirection = FVector::ForwardVector;

	/** 수평 넉백 힘 */
	UPROPERTY()
	float KnockbackForce = 400.0f;

	/** 수직(상향) 넉백 힘 */
	UPROPERTY()
	float KnockbackUpForce = 200.0f;

	UPROPERTY()
	EHitInstigatorType HitInstigator = EHitInstigatorType::None;

	/** 공격을 가한 액터 (무기 소유 캐릭터, 취객 NPC 등). 수신 측의 "누가 때렸는지" 판정용. 기믹류는 null일 수 있다 */
	UPROPERTY()
	TObjectPtr<AActor> HitInstigatorActor = nullptr;

	UPROPERTY()
	EHitReactionType HitReaction = EHitReactionType::None;

	UPROPERTY()
	FVector ImpactPoint = FVector::ZeroVector;

	UPROPERTY()
	float ExplosionRadius = 0.0f;

	UPROPERTY()
	float ExplosionStrength = 0.0f;
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCombatReceiver : public UInterface
{
	GENERATED_BODY()
};

class TROMBONERUMBLE_API ICombatReceiver
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/** 피격 처리를 시도한다.
	 *  @return true = 리액션이 적용됨(None 포함), false = 상태 게이트(무적/스턴/래그돌)나 권한 부재로 거부됨.
	 *          호출자(방해 NPC의 포획 등)가 "이미 무력화된 대상"을 구분해 후속 행동(타겟 변경 등)을 결정할 수 있다. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	bool OnHitReceived(const FHitData& HitData);
};
