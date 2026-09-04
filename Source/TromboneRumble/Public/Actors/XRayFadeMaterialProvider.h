// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "XRayFadeMaterialProvider.generated.h"

class UXRayFadeMaterialMap;

/**
 * X-Ray 디더 페이드용 머티리얼 맵을 레벨이 소유하게 하는 홀더 액터.
 *
 * 로직이 없다. 존재 이유는 오직 하나 — 맵을 하드 참조해서 페이드 사본 머티리얼들이
 * 레벨 패키지와 함께 로드되게 하는 것이다. 덕분에 게임 도중 추가 로딩이 0이고
 * 비용은 로딩 화면에 흡수된다.
 *
 * 맵은 레벨마다 따로 있다. 가림 처리를 쓰는 레벨마다 하나씩 배치하고
 * 그 레벨의 FadeMaterialMap을 지정해야 한다.
 * 없으면 UXRayTranslucentFadeComponent가 경고를 남기고 잠든 채로 남는다.
 */
UCLASS()
class TROMBONERUMBLE_API AXRayFadeMaterialProvider : public AActor
{
	GENERATED_BODY()

public:

	AXRayFadeMaterialProvider();

	/** 레벨에 배치된 첫 번째 제공자의 맵을 반환. 없으면 nullptr, 2개 이상이면 경고 로그 */
	static UXRayFadeMaterialMap* FindMapInLevel(const UObject* WorldContextObject);

protected:

	/** Tools/generate_xray_fade_materials.py 가 생성한 DA_XRayFadeMaterialMap_<레벨명>. 이 레벨 것을 지정할 것 */
	UPROPERTY(EditAnywhere, Category = "XRay", meta = (DisplayName = "페이드 머티리얼 맵"))
	TObjectPtr<UXRayFadeMaterialMap> FadeMaterialMap;
};
