// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponents/XRayComponentBase.h"
#include "XRaySilhouetteComponent.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class UPrimitiveComponent;

/**
 * X-Ray 방식 ①: 가려진 캐릭터를 벽 너머로 단색 실루엣으로 비춘다.
 *
 * 캐릭터와 장착물의 메시에 CustomDepth 스텐실 값을 찍고, PostProcess 머티리얼
 * (M_XRaySilhouette)이 그 스텐실을 읽어 실루엣을 그린다.
 *
 * 스텐실은 이 컴포넌트가 트레이스 주기마다 일괄 관리한다. 캐릭터가 머리(리더 메시)만 있고
 * 안테나/코스튬은 UCustomizationComponent가 런타임에 붙였다 뗐다 하는 구조라, 매번 현재
 * 붙어있는 메시를 다시 훑어야 커마 변경이 자동으로 반영된다.
 * 컴포넌트를 떼면 스텐실 렌더링도 같이 꺼진다 (안 쓰는데 CustomDepth 비용만 내는 상황 방지).
 */
UCLASS(ClassGroup=(XRay), meta=(BlueprintSpawnableComponent))
class TROMBONERUMBLE_API UXRaySilhouetteComponent : public UXRayComponentBase
{
	GENERATED_BODY()

public:
	/** 단일 프리미티브에 X-Ray 실루엣용 CustomDepth 스텐실 값 설정 */
	static void ApplyOccludedStencil(UPrimitiveComponent* Prim);
	static void ClearOccludedStencil(UPrimitiveComponent* Prim);

protected:
	virtual bool InitializeEffect() override;
	virtual void UpdateEffect(float DeltaTime) override;
	virtual void OnTraceUpdated(const TArray<AActor*>& Occluders) override;
	virtual void TeardownEffect() override;

	/** 실루엣 색은 캐릭터 피부색을 따라간다 (베이스가 ATromboneCharacterBase::OnSkinColorChanged를 구독해 넘겨준다) */
	virtual void OnSkinColorChanged(const FLinearColor& NewSkinColor) override;

	/** 실루엣 PostProcess 머티리얼 (M_XRaySilhouette). 비어 있으면 이 컴포넌트는 동작하지 않는다 */
	UPROPERTY(EditAnywhere, Category = "XRay|Silhouette")
	TObjectPtr<UMaterialInterface> SilhouetteMaterial;

private:
	void SetSilhouetteColor(const FLinearColor& InColor);

	/** 오너 + 부착 액터들의 메시에 스텐실을 갱신하고, 떠난 액터의 스텐실은 해제 */
	void RefreshStencilTargets();
	void ClearAllStencils();
	void SetBlendableWeight(float Weight);

	static void ApplyStencilToActor(AActor* Actor);
	static void ClearStencilFromActor(AActor* Actor);

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> SilhouetteMID;

	// 지금 스텐실이 찍혀 있는 액터들. 다음 갱신 때 여기서 빠진 액터는 스텐실을 해제한다
	TSet<TWeakObjectPtr<AActor>> StenciledActors;

	bool bSilhouetteVisible = false;
};
