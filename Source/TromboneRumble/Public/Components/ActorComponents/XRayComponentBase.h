// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "XRayComponentBase.generated.h"

class AController;
class UCameraComponent;

/**
 * 카메라→플레이어 사이가 가려졌을 때의 시각 처리 공통 베이스.
 *
 * 파생 컴포넌트를 액터에 붙이거나 떼는 것만으로 X-Ray 방식을 갈아끼울 수 있다.
 * 여러 개를 동시에 붙여도 서로 간섭하지 않지만(각자 자기 리소스만 관리) 효과는 겹쳐 보인다.
 *
 * 베이스가 담당하는 것: 자체 초기화 + 카메라→오너 스피어 스윕(TraceInterval 주기) + OccluderTag 필터.
 * 파생이 담당하는 것: 실제 시각 효과 (InitializeEffect/UpdateEffect/OnTraceUpdated/TeardownEffect).
 *
 * 로컬 플레이어 전용(복제 없음). 오너가 로컬 조종 폰일 때만 스스로 켜지며, 캐릭터 쪽에서
 * 해줘야 할 일은 없다. 클라이언트에서 possession이 늦게 오면 컨트롤러 변경 알림을 받아 재시도한다.
 */
UCLASS(Abstract, ClassGroup=(XRay))
class TROMBONERUMBLE_API UXRayComponentBase : public UActorComponent
{
	GENERATED_BODY()

public:
	UXRayComponentBase();

	/**
	 * 효과를 일시 정지/재개한다 (치트·벤치마크에서 한 번에 한 방식만 켜기 위한 용도).
	 * 정지 시 효과 리소스를 완전히 해제하므로 화면에 아무 흔적도 남지 않는다.
	 */
	void SetTrackingPaused(bool bPaused);

	bool IsTrackingPaused() const { return bTrackingPaused; }

	/** 이번 스윕에서 가리는 액터가 하나라도 있었는지 */
	bool IsAnyOccluding() const { return bAnyOccluding; }

	/** 효과 초기화까지 성공해 실제로 동작 중인지 */
	bool IsEffectActive() const { return bEffectActive; }

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 효과 리소스 생성. false를 반환하면 이 컴포넌트는 잠든 상태로 남는다. 재호출(재개) 가능해야 함. */
	virtual bool InitializeEffect() { return true; }

	/** 매 프레임. 보간/파라미터 갱신용 */
	virtual void UpdateEffect(float DeltaTime) {}

	/** TraceInterval 주기. Occluders는 태그 필터를 통과한 가림 액터 목록 */
	virtual void OnTraceUpdated(const TArray<AActor*>& Occluders) {}

	/** 효과 리소스 해제 + 화면 원상복구. 여러 번 불려도 안전해야 함 */
	virtual void TeardownEffect() {}

	/** 오너 캐릭터의 피부색이 바뀌었을 때. 색이 필요한 파생만 구현하면 된다 */
	virtual void OnSkinColorChanged(const FLinearColor& NewSkinColor) {}

	const UCameraComponent* GetCamera() const { return Camera.Get(); }
	UCameraComponent* GetCamera() { return Camera.Get(); }

	/** 오너를 조종 중인 로컬 플레이어 컨트롤러 (없으면 null) */
	const APlayerController* GetOwningPlayerController() const;

	// 카메라→플레이어 스윕 주기 (효과 보간은 매 프레임)
	UPROPERTY(EditAnywhere, Category = "XRay")
	float TraceInterval = 0.1f;

	// 스윕 스피어 반경. 라인 트레이스는 벽 모서리에서 깜빡여서 캐릭터 실루엣 근사치로 스피어 사용
	UPROPERTY(EditAnywhere, Category = "XRay")
	float TraceSphereRadius = 40.f;

	// true면 OccluderTag가 붙은 액터만 대상, false면 스윕에 걸린 모든
	// WorldStatic/WorldDynamic/PhysicsBody 액터 대상. Pawn은 쿼리에 없다
	UPROPERTY(EditAnywhere, Category = "XRay")
	bool bRequireOccluderTag = true;

	UPROPERTY(EditAnywhere, Category = "XRay")
	FName OccluderTag = FName(TEXT("XRayBlocker"));

private:
	/** 오너가 로컬 조종 폰이면 카메라를 찾아 추적을 시작. 조건이 안 맞으면 조용히 대기한다 */
	void TryStartTracking();

	/** 효과 초기화에 실패하면 조용히 잠든 채로 남는다 (다른 X-Ray 컴포넌트를 방해하지 않기 위함) */
	void StartOcclusionTracking(UCameraComponent* InCamera);

	/** 클라이언트에서 possession이 늦게 오는 경우의 재시도 경로 */
	UFUNCTION()
	void HandleControllerChanged(APawn* InPawn, AController* OldController, AController* NewController);

	UFUNCTION()
	void HandleSkinColorChanged(const FLinearColor& NewSkinColor);

	void UpdateTrace();

	TWeakObjectPtr<UCameraComponent> Camera;
	float TimeSinceTrace = 0.f;
	bool bAnyOccluding = false;
	bool bEffectActive = false;
	bool bTrackingPaused = false;
};
