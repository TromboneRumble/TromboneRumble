// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponents/XRayComponentBase.h"
#include "XRayTranslucentFadeComponent.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class UMeshComponent;
class UXRayFadeMaterialMap;

/**
 * X-Ray 방식 ②: 카메라와 플레이어(본인 + 화면 안의 다른 플레이어) 사이를 가리는 액터를 통째로 Translucent 반투명 처리
 */
UCLASS(ClassGroup=(XRay), meta=(BlueprintSpawnableComponent))
class TROMBONERUMBLE_API UXRayTranslucentFadeComponent : public UXRayComponentBase
{
	GENERATED_BODY()

protected:
	virtual bool InitializeEffect() override;
	virtual void UpdateEffect(float DeltaTime) override;
	virtual void OnTraceUpdated(const TArray<AActor*>& Occluders) override;
	virtual void TeardownEffect() override;
	virtual void GatherTraceTargets(TArray<AActor*>& OutTargets) const override;

	// 완전히 페이드됐을 때 남는 불투명도 (사본 머티리얼의 FadedOpacity 파라미터로 주입)
	UPROPERTY(EditAnywhere, Category = "XRay|Dither", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FadedOpacity = 0.35f;

	UPROPERTY(EditAnywhere, Category = "XRay|Dither")
	float FadeInterpSpeed = 6.f;

	// 다른 플레이어 캐릭터가 가림물 뒤에 있어도 페이드. 끄면 본인만
	UPROPERTY(EditAnywhere, Category = "XRay|Dither")
	bool bFadeForOtherPlayers = true;

	// 화면 밖 플레이어는 제외. 뷰포트 크기 대비 여유 비율 (가장자리 깜빡임 완화)
	UPROPERTY(EditAnywhere, Category = "XRay|Dither", meta = (ClampMin = "0.0", ClampMax = "0.5", EditCondition = "bFadeForOtherPlayers"))
	float OtherPlayerScreenMargin = 0.1f;

private:
	/** 교체 대상 슬롯 하나. 원본을 기억해야 페이드가 끝났을 때 되돌릴 수 있다 */
	struct FFadeSlot
	{
		TWeakObjectPtr<UMeshComponent> Mesh;
		int32 SlotIndex = INDEX_NONE;
		TWeakObjectPtr<UMaterialInterface> Original;
		// 소유권은 FadeMIDs가 갖는다 (아래 주석 참고)
		TWeakObjectPtr<UMaterialInstanceDynamic> FadeMID;
	};

	struct FOccluderState
	{
		TArray<FFadeSlot> Slots;
		float CurrentFade = 0.f;
		bool bOccluding = false;
		bool bSlotsInitialized = false;
		// 지금 사본이 메시에 물려 있는지. 페이드가 0으로 끝나면 원본으로 되돌리고 false
		bool bBound = false;
	};

	void EnsureFadeSlots(const AActor* Actor, FOccluderState& State);
	void BindFadeMaterials(FOccluderState& State);
	void RestoreOriginalMaterials(FOccluderState& State);

	/** 레벨의 AXRayFadeMaterialProvider가 들고 있는 매핑. 사본들은 레벨과 함께 이미 로드돼 있다 */
	UPROPERTY()
	TObjectPtr<UXRayFadeMaterialMap> FadeMap;

	/**
	 * 생성한 MID의 GC 소유권. 예전 구현은 MID를 만들자마자 메시에 물려서 메시의
	 * OverrideMaterials(UPROPERTY)가 잡아줬지만, 지금은 가려질 때까지 물리지 않으므로
	 * 여기서 직접 붙들지 않으면 수거된다.
	 */
	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> FadeMIDs;

	TMap<TWeakObjectPtr<AActor>, FOccluderState> Occluders;
};
