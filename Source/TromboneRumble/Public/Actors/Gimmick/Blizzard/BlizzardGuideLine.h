// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Actors/Gimmick/Blizzard/BlizzardGimmick.h"
#include "BlizzardGuideLine.generated.h"

class ABlizzardShelter;
class UNiagaraComponent;

/** ABlizzardGuideLine
 * 열린 천막까지 이어지는 안내선. 스플라인을 따라 나이아가라가 파티클을 계속 흘려보낸다.
 *
 * 켜지는 조건은 두 사실의 AND 다:
 *   - 목표 천막의 문이 열렸다 (= 이번 전조에 뽑힌 천막이다)
 *   - 지금이 전조(Warning) 구간이다
 *
 * 두 사실은 서로 다른 액터에서 따로 복제되고 도착 순서가 보장되지 않는다. 서버에서도
 * 순서가 뒤집혀 있다 - StartWarning() 은 SetState(Warning) 을 먼저 부르고 문은 나중에 연다
 * (BlizzardGimmick.cpp). 그래서 두 델리게이트를 모두 구독하고, 어느 쪽이 오든
 * RefreshGuideLine() 에서 조건을 다시 계산한다. 한쪽만 구독하면 반드시 깨진다.
 *
 * 복제하지 않는다. 각 머신이 복제된 쉘터/기믹 상태를 보고 스스로 켜고 끈다.
 *
 * 스플라인과 나이아가라는 C++ 이 아니라 BP 가 소유한다 (아트가 배치한
 * BP_SplineVFX_EnergyLoop 를 이 클래스로 리페어런트했다).
 * 네이티브 컴포넌트를 추가하지 말 것 - BP 의 루트가 한 단 밀린다.
 */
UCLASS()
class TROMBONERUMBLE_API ABlizzardGuideLine : public AActor
{
	GENERATED_BODY()

public:
	ABlizzardGuideLine();

	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** 이 안내선이 가리키는 천막. 레벨에서 1:1 로 손수 짝지어 준다 (반경 검색이 아니다).
	 *  Transient 를 붙이면 레벨 저장 시 참조가 날아간다 - 천막 문 액터와 같은 이유. */
	UPROPERTY(EditInstanceOnly, Category = "Config|GuideLine", meta = (AllowPrivateAccess = "true", DisplayName = "목표 천막"))
	TSoftObjectPtr<ABlizzardShelter> TargetShelter;

	/** 상태를 받아올 눈보라 기믹. 레벨에서 직접 지정한다.
	 *  비워두면 이 안내선만 켜지지 않는다 (BeginPlay 에 경고). */
	UPROPERTY(EditInstanceOnly, Category = "Config|GuideLine", meta = (AllowPrivateAccess = "true", DisplayName = "눈보라 기믹"))
	TWeakObjectPtr<ABlizzardGimmick> BlizzardGimmick;

	/** 기믹의 OnBlizzardStateChangedDelegate 구독 핸들러. */
	UFUNCTION()
	void HandleBlizzardStateChanged(EBlizzardState NewState);

	/** 목표 천막의 OnDoorChanged 구독 핸들러. */
	UFUNCTION()
	void HandleShelterDoorChanged(bool bIsOpen);

	/** 두 신호를 합쳐 켜고 끈다. 어느 쪽이 와도 여기서 전부 다시 계산한다. */
	void RefreshGuideLine();

	/** BP 가 만든 나이아가라. 월드 소유라 약참조. */
	TWeakObjectPtr<UNiagaraComponent> GuideFX;

	/** 해석해 둔 목표 천막. EndPlay 에서 구독을 풀 때도 쓴다. */
	TWeakObjectPtr<ABlizzardShelter> ResolvedShelter;

	EBlizzardState CachedState = EBlizzardState::Idle;
	bool bDoorOpen = false;

	/** 지금 켜져 있는지. 전조에서 눈보라로 넘어갈 때만 부드럽게 끄기 위해 필요하다. */
	bool bShowing = false;
};
