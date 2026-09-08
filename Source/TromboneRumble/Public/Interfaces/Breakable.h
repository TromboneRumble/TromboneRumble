// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Utilities/Defines.h"
#include "Breakable.generated.h"

/** 파괴 한 번에 대한 정보. 복제되어 각 머신이 같은 방향으로 파편을 날린다 */
USTRUCT(BlueprintType)
struct FBreakHitInfo
{
	GENERATED_BODY()

	/** 파편이 터져나갈 중심. 월드 좌표 */
	UPROPERTY()
	FVector_NetQuantize ImpactPoint = FVector::ZeroVector;

	/** 파편이 날아갈 방향 (정규화) */
	UPROPERTY()
	FVector_NetQuantizeNormal ImpactDirection = FVector::ForwardVector;

	/** 파편 초기 속도의 크기 (cm/s) */
	UPROPERTY()
	float Strength = 300.f;

	UPROPERTY()
	EBreakSource Source = EBreakSource::Script;

	/** 부순 주체 (플레이어, 취객 NPC 등). 로컬 판정용이라 복제하지 않는다 */
	UPROPERTY(NotReplicated)
	TObjectPtr<AActor> Instigator = nullptr;
};

UINTERFACE(MinimalAPI)
class UBreakable : public UInterface
{
	GENERATED_BODY()
};

/** 부술 수 있는 소품(술잔, 문 등)이 구현한다. Chaos GeometryCollection 연출은 구현체가 알아서 한다 */
class TROMBONERUMBLE_API IBreakable
{
	GENERATED_BODY()

public:
	/** 파괴를 시도한다. 서버 전용.
	 *  @return true = 이번 호출로 부서짐, false = 이미 부서졌거나 이 경로(닿기/공격)를 허용하지 않음 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Breakable")
	bool Break(const FBreakHitInfo& HitInfo);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Breakable")
	bool IsBroken() const;
};
