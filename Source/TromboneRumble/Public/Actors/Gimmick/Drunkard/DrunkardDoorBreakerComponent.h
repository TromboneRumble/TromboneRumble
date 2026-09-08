// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/Breakable.h"
#include "DrunkardDoorBreakerComponent.generated.h"

class ABreakableDoor;
class ADrunkardNPC;

/** 스폰 지점 하나 ↔ 그 지점에서 취객이 뚫고 나올 문 하나 */
USTRUCT(BlueprintType)
struct FDrunkardDoorBinding
{
	GENERATED_BODY()

	/** 스포너의 "문 목록"(Doors)에 들어 있는 스폰 지점 액터 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (DisplayName = "스폰 지점"))
	TObjectPtr<AActor> SpawnPoint = nullptr;

	/** 그 지점에서 부술 문 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (DisplayName = "문"))
	TObjectPtr<ABreakableDoor> Door = nullptr;
};

/** UDrunkardDoorBreakerComponent
 *
 * ADrunkardSpawner 생성자가 네이티브 서브오브젝트로 달아준다. 취객이 스폰되면 그 스폰 지점에 짝지어 둔 문을 부순다. 서버 전용.
 *
 * AGimmickManager와 무관하고, 스포너는 이 컴포넌트를 만들기만 할 뿐 문·파괴 로직은 전혀 모른다 —
 * 컴포넌트가 공개된 OnDrunkardSpawned 델리게이트를 스스로 구독한다.
 * 스포너의 Doors 배열은 이름과 달리 스폰 지점(TargetPoint)이다. 실제 문은 DoorBindings가 1:1로 가리킨다.
 * 충격은 그 문의 GeometryCollection에만 직접 적용하므로 취객·플레이어·다른 소품에는 영향이 없다.
 */
UCLASS(ClassGroup = (Gimmick), meta = (BlueprintSpawnableComponent))
class TROMBONERUMBLE_API UDrunkardDoorBreakerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDrunkardDoorBreakerComponent();

protected:
	/** 레벨의 스포너 인스턴스를 골라 이 컴포넌트에서 지정한다. 스포너 "문 목록"의 지점마다 한 쌍.
	 *  비워 두면 문 파괴를 건너뛴다 — 부술 문이 없는 맵(블록아웃 등)에서는 그게 정상이다 */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Config", meta = (DisplayName = "스폰 지점 - 문 바인딩"))
	TArray<FDrunkardDoorBinding> DoorBindings;

	/** 스폰 후 파괴까지 지연 (초). 음수면 취객이 문 평면을 지나는 시점 = 문 통과 시간의 절반 */
	UPROPERTY(EditAnywhere, Category = "Config", meta = (DisplayName = "파괴 지연"))
	float BreakDelaySeconds = -1.f;

	/** 파편 초기 속도 (cm/s) */
	UPROPERTY(EditAnywhere, Category = "Config", meta = (DisplayName = "파편 초속", ClampMin = "0.0"))
	float BreakStrength = 600.f;

	/** 짝지은 지점과 문이 이보다 멀면 잘못 연결한 것으로 보고 경고를 찍는다 (cm) */
	UPROPERTY(EditAnywhere, Category = "Config", meta = (DisplayName = "바인딩 거리 경고", ClampMin = "0.0"))
	float BindingWarnDistance = 500.f;

	/** 스포너가 넘겨주는 두 번째 인자는 이름이 Door지만 실제로는 스폰 지점이다 */
	UFUNCTION()
	void HandleDrunkardSpawned(ADrunkardNPC* NPC, AActor* SpawnPoint);

	void BreakDoor(TWeakObjectPtr<ABreakableDoor> Door, FBreakHitInfo HitInfo);

	const FDrunkardDoorBinding* FindBinding(const AActor* SpawnPoint) const;

	/** 배치 실수(빈 칸, 엉뚱한 짝)를 첫 PIE에서 드러낸다 */
	void ValidateBindings() const;

private:
	FTimerHandle BreakTimerHandle;

public:
	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End UActorComponent Interface
};
