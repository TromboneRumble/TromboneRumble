// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponents/XRayComponentBase.h"
#include "XRayWindowComponent.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;

/**
 * X-Ray 방식 ③: 가려졌을 때 캐릭터 주변에 원형 윈도우를 뚫어 "가리는 물체가 없는 세계"를 합성한다.
 *
 * SceneCapture2D가 씬 전체를 찍되 가리는 액터만 HiddenActors로 빼고 렌더타겟에 담고,
 * PostProcess 머티리얼(M_PP_OcclusionWindow)이 원 안에서 "가리는 물체가 그려진 픽셀"에만 그것을 합성한다.
 * 가리는 액터에 CustomDepth 스텐실(OCCLUDER_STENCIL)을 찍어 머티리얼이 그 픽셀을 찾는다.
 * 나머지 픽셀은 원본 화면 그대로라 캡처와 화면의 렌더 차이(GI/AO 등)가 드러나지 않는다.
 * 환경 머티리얼을 전혀 수정하지 않아도 되고, 가리는 액터에 XRayBlocker 태그만 있으면 동작한다.
 * 대신 가려진 동안에는 캡처용 씬 렌더 패스가 한 번 더 돈다 (평소에는 패스 자체가 제거됨).
 */
UCLASS(ClassGroup=(XRay), meta=(BlueprintSpawnableComponent))
class TROMBONERUMBLE_API UXRayWindowComponent : public UXRayComponentBase
{
	GENERATED_BODY()

public:
	/**
	 * 런타임 튜닝용 (Trombone_XRayWindow 치트). BP를 다시 컴파일하지 않고 크기/부드러움을 잡을 때 쓴다.
	 * 여기서 바꾼 값은 저장되지 않으므로, 마음에 드는 값은 BP의 XRay|Window 카테고리에 옮겨 적어야 한다.
	 */
	void SetWindowShape(float InWorldHoleRadius, float InEdgeSoftness, float InOccluderOpacity);

	float GetWorldHoleRadius() const { return WorldHoleRadius; }
	float GetEdgeSoftness() const { return EdgeSoftness; }
	float GetOccluderOpacity() const { return OccluderOpacity; }

	/** 크롭 on/off 실측 비교용 (Trombone_XRayCropCapture 치트). 렌더타겟을 다시 만들어야 해서 재초기화가 필요하다 */
	bool IsCropCaptureEnabled() const { return bCropCaptureToWindow; }
	void SetCropCaptureEnabled(bool bEnabled) { bCropCaptureToWindow = bEnabled; }

protected:
	virtual bool InitializeEffect() override;
	virtual void UpdateEffect(float DeltaTime) override;
	virtual void OnTraceUpdated(const TArray<AActor*>& Occluders) override;
	virtual void TeardownEffect() override;

	/** 합성용 PostProcess 머티리얼 (M_PP_OcclusionWindow). 비어 있으면 이 컴포넌트는 동작하지 않는다 */
	UPROPERTY(EditAnywhere, Category = "XRay|Window")
	TObjectPtr<UMaterialInterface> WindowMaterial;

	/**
	 * 캡처 시야를 화면 전체가 아니라 "원을 감싸는 사각형"으로 좁힌다.
	 * 합성에 쓰이는 픽셀은 원 안뿐이라 나머지는 그려도 버려진다. 좁히면 픽셀 비용은 물론
	 * 프러스텀 컬링으로 드로우콜/지오메트리 비용까지 함께 줄어든다.
	 * 끄면 아래 CaptureResolutionScale을 쓰는 기존 전체 화면 캡처로 돌아간다 (성능 비교용).
	 */
	UPROPERTY(EditAnywhere, Category = "XRay|Window")
	bool bCropCaptureToWindow = true;

	// 크롭 모드의 렌더타겟 한 변 크기 (px). 원은 화면에서 대체로 정사각형이라 정사각 RT를 쓴다
	UPROPERTY(EditAnywhere, Category = "XRay|Window", meta = (ClampMin = "64", ClampMax = "2048", EditCondition = "bCropCaptureToWindow"))
	int32 CropRTSize = 512;

	/**
	 * 크롭 사각형을 원 반경보다 얼마나 크게 잡을지. 가장자리 그라데이션과 바이리니어 여유분.
	 * 1 미만으로 내리면 원 가장자리가 캡처 밖으로 나가 잘린다
	 */
	UPROPERTY(EditAnywhere, Category = "XRay|Window", meta = (ClampMin = "1.0", ClampMax = "2.0", EditCondition = "bCropCaptureToWindow"))
	float CropMarginFactor = 1.2f;

	// 전체 화면 캡처(크롭 off)일 때의 렌더타겟 해상도 = 뷰포트 크기 × 이 값
	UPROPERTY(EditAnywhere, Category = "XRay|Window", meta = (ClampMin = "0.1", ClampMax = "1.0", EditCondition = "!bCropCaptureToWindow"))
	float CaptureResolutionScale = 0.5f;

	// 캐릭터 주변에 뚫을 원의 월드 반경 (cm). 화면 반경은 카메라 거리에 따라 자동 계산
	UPROPERTY(EditAnywhere, Category = "XRay|Window")
	float WorldHoleRadius = 120.f;

	// 원 가장자리 그라데이션 폭. 0이면 칼같이 잘린 경계, 1이면 매우 부드럽게 풀어진다
	UPROPERTY(EditAnywhere, Category = "XRay|Window", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EdgeSoftness = 0.45f;

	// 원 안에서 가리는 물체가 남아 보이는 정도. 0=완전히 사라짐, 1=그대로(효과 없음).
	// 캡처로 덮는 비율을 (1 - 이 값)으로 낮춰서 원래 화면이 그만큼 비쳐 보이게 한다
	UPROPERTY(EditAnywhere, Category = "XRay|Window", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float OccluderOpacity = 0.15f;

	// 원 안의 화면을 얼마나 어둡게 깔지 (1=그대로, 0=완전 검정).
	// 가리는 물체가 있던 자리는 캡처가 덮으므로 그 바깥 영역에만 영향을 준다
	UPROPERTY(EditAnywhere, Category = "XRay|Window", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WallDim = 1.f;

	// 원이 중앙에서 최대 크기까지 확장되는 데 걸리는 시간 (초)
	UPROPERTY(EditAnywhere, Category = "XRay|Window", meta = (ClampMin = "0.01"))
	float OpenDuration = 0.18f;

	// 가림이 풀렸을 때 원이 중앙으로 수축하는 데 걸리는 시간 (초)
	UPROPERTY(EditAnywhere, Category = "XRay|Window", meta = (ClampMin = "0.01"))
	float CloseDuration = 0.25f;

	// 열릴 때 최종 크기를 얼마나 넘겼다 돌아올지. 0이면 오버슈트 없이 매끈하게 커진다
	UPROPERTY(EditAnywhere, Category = "XRay|Window", meta = (ClampMin = "0.0", ClampMax = "3.0"))
	float PopOvershoot = 1.4f;

	// 캡처에서 그림자를 렌더할지. 끄면 윈도우 안 캐릭터가 평평해 보이지만 GPU 비용 감소
	UPROPERTY(EditAnywhere, Category = "XRay|Window")
	bool bCaptureShadows = true;

private:
	/** 캐시해둔 가림 액터를 SceneCapture의 HiddenActors로 밀어 넣고 스텐실도 같이 갱신한다 */
	void ApplyHiddenActors();

	/** 머티리얼이 "가리는 물체가 그려진 픽셀"을 찾을 수 있도록 CustomDepth 스텐실을 찍는다 */
	void RefreshOccluderStencils();
	void ClearOccluderStencils();

	void SetCaptureActive(bool bActive);
	void SetBlendableWeight(float Weight);

	/** 캡처 투영과 머티리얼의 CaptureRect를 이번 프레임 원 위치에 맞춰 함께 갱신한다 */
	void UpdateCaptureCrop(const class APlayerController* PC, const FVector2D& CenterUV, float RadiusUV, float ViewAspect);

	/** 이번 프레임 캡처가 담을 화면 영역을 계산한다. 원이 통째로 화면 밖이면 false */
	bool ComputeCaptureRect(const FVector2D& CenterUV, float RadiusUV, float ViewAspect, FVector2D& OutMin, FVector2D& OutSize) const;

	/**
	 * 캡처 시야를 화면의 일부(RectMin ~ RectMin+RectSize, UV 단위)로 좁힌 투영 행렬을 만든다.
	 * 카메라의 실제 투영에 클립 공간 크롭을 곱하는 방식이라 FOV/종횡비/근평면을 그대로 물려받는다.
	 * 렉트가 (0,0)-(1,1)이면 원본 투영과 같다.
	 */
	static FMatrix BuildCroppedProjection(const FMatrix& CameraProjection, const FVector2D& RectMin, const FVector2D& RectSize);

	// 렉트 중심이 한 프레임에 이만큼(UV) 넘게 튀면 카메라 컷으로 보고 캡처 히스토리를 버린다
	static constexpr float RectJumpThreshold = 0.25f;

	// 이번 스윕에서 잡힌 가림 액터. 캡처에서 이것들만 빼고 씬 전체를 그린다
	TArray<TWeakObjectPtr<AActor>> CachedOccluders;

	// 지금 스텐실이 찍혀 있는 액터들. 목록에서 빠지면 해제한다
	TSet<TWeakObjectPtr<AActor>> StenciledOccluders;

	UPROPERTY(Transient)
	TObjectPtr<USceneCaptureComponent2D> SceneCapture;

	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> CaptureRT;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> WindowMID;

	float WindowFade = 0.f;
	bool bCaptureActive = false;
	FIntPoint LastViewportSize = FIntPoint::ZeroValue;

	// 크롭 렉트가 한 번에 크게 튀면(카메라 컷/텔레포트) 캡처의 오클루전 히스토리를 버려야 한다
	FVector2D LastRectCenter = FVector2D::ZeroVector;
};
