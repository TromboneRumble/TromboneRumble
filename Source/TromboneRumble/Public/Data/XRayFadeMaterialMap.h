// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "XRayFadeMaterialMap.generated.h"

class UMaterialInterface;

/**
 * X-Ray 디더 페이드용 원본 → 페이드 사본 머티리얼 매핑.
 *
 * 원본 머티리얼은 손대지 않고, MF_OcclusionFade를 심은 사본을 따로 만들어 두었다가
 * UXRayTranslucentFadeComponent가 가려지는 동안만 메시 슬롯을 사본으로 갈아끼운다.
 *
 * 레벨마다 하나씩 있다 (DA_XRayFadeMaterialMap_<레벨명>). 그래야 한 레벨을 다시 만들어도
 * 다른 레벨 매핑이 남고, 그 레벨에 필요한 사본만 로드된다.
 *
 * 이 에셋은 Tools/generate_xray_fade_materials.py 가 통째로 다시 쓴다. 손으로 편집하지 말 것.
 */
UCLASS()
class TROMBONERUMBLE_API UXRayFadeMaterialMap : public UDataAsset
{
	GENERATED_BODY()

public:

	/** 원본에 대응하는 사본을 반환. 페이드 대상이 아니면 nullptr */
	UMaterialInterface* FindFadeVariant(UMaterialInterface* Original) const;

	/** 사본 전체 (PSO 예열용) */
	void GetAllFadeVariants(TArray<UMaterialInterface*>& OutVariants) const;

	/**
	 * 원본 → 페이드 사본. 하드 참조라서 이 에셋이 올라올 때 사본들도 같이 올라온다.
	 * 덕분에 게임 도중 추가 로딩이 없고, 쿡 설정(DirectoriesToAlwaysCook)도 필요 없다.
	 */
	UPROPERTY(EditAnywhere, Category = "XRay", meta = (DisplayName = "원본 → 페이드 사본"))
	TMap<TObjectPtr<UMaterialInterface>, TObjectPtr<UMaterialInterface>> FadeVariants;
};
