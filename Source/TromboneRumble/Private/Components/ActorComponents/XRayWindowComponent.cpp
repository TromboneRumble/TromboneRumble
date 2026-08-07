// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Components/ActorComponents/XRayWindowComponent.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/MeshComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/PlayerController.h"
#include "SceneView.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Utilities/Defines.h"

namespace
{
	// 뷰포트 크기를 아직 못 얻는 시점(BeginPlay 직후)에 쓸 임시 RT 크기. 첫 활성화 때 실제 크기로 리사이즈됨
	constexpr int32 FallbackRTSize = 512;

	/**
	 * 0→1 진행도를 "튀어나오는" 곡선으로 바꾼다 (EaseOutBack).
	 * 초반이 매우 빠르고 1을 살짝 넘겼다가 제자리로 돌아와서, 원이 캐릭터 중앙에서
	 * 순간적으로 확장되는 체감을 준다. T=0에서 0, T=1에서 정확히 1을 반환한다.
	 */
	float EaseOutBack(float T, float Overshoot)
	{
		const float U = T - 1.f;
		return 1.f + (Overshoot + 1.f) * U * U * U + Overshoot * U * U;
	}
}

bool UXRayWindowComponent::InitializeEffect()
{
	AActor* Owner = GetOwner();
	UCameraComponent* Cam = GetCamera();
	if (!Owner || !Cam)
	{
		return false;
	}

	UMaterialInterface* Material = WindowMaterial;
	if (!Material)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[XRayWindow] 합성 머티리얼이 없습니다. BP의 XRay|Window > WindowMaterial 에 "
			     "M_PP_OcclusionWindow 를 지정하세요."));
		return false;
	}

	int32 SizeX = FallbackRTSize;
	int32 SizeY = FallbackRTSize;
	if (bCropCaptureToWindow)
	{
		// 캡처 시야를 원 주변으로 좁히므로 RT는 화면 크기와 무관하다.
		// 크기가 고정이라 창 크기가 바뀌어도 리사이즈할 일이 없다
		SizeX = SizeY = FMath::Max(1, CropRTSize);
	}
	else if (const APlayerController* PC = GetOwningPlayerController())
	{
		int32 ViewX = 0, ViewY = 0;
		PC->GetViewportSize(ViewX, ViewY);
		if (ViewX > 0 && ViewY > 0)
		{
			LastViewportSize = FIntPoint(ViewX, ViewY);
			SizeX = FMath::Max(1, FMath::RoundToInt(ViewX * CaptureResolutionScale));
			SizeY = FMath::Max(1, FMath::RoundToInt(ViewY * CaptureResolutionScale));
		}
	}

	// 캡처가 매 프레임 전체를 덮어쓰므로 ClearColor는 첫 프레임에만 의미가 있다.
	// 머티리얼은 캡처 알파를 쓰지 않고 원 마스크만으로 합성한다
	CaptureRT = UKismetRenderingLibrary::CreateRenderTarget2D(
		this, SizeX, SizeY, RTF_RGBA16f, FLinearColor(0.f, 0.f, 0.f, 1.f));
	if (!CaptureRT)
	{
		return false;
	}

	SceneCapture = NewObject<USceneCaptureComponent2D>(Owner);
	SceneCapture->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
	// 화이트리스트가 아니라 블랙리스트. 씬 전체를 그리되 HiddenActors(=가리는 액터)만 뺀다.
	// ShowOnlyActors는 채우지 않는다 — 이 모드에서 남아 있으면 렌더러가 무시하면서 경고를 찍는다
	SceneCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
	SceneCapture->TextureTarget = CaptureRT;
	SceneCapture->bCaptureEveryFrame = true;
	SceneCapture->bCaptureOnMovement = false;
	SceneCapture->FOVAngle = Cam->FieldOfView;
	SceneCapture->RegisterComponent();
	SceneCapture->AttachToComponent(Cam, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	// ShowFlags는 반드시 RegisterComponent 이후에. OnRegister→UpdateShowFlags가 아키타입 값으로 되돌린다.
	// AO/반사/포그는 끄지 않는다 — 캡처가 메인 화면과 같은 기능으로 그려져야 원 안팎 색이 맞는다
	SceneCapture->ShowFlags.SetDynamicShadows(bCaptureShadows);

	// 가려지기 전까지는 캡처 자체를 돌리지 않는다 (CaptureSceneDeferred가 IsVisible 검사로 early-out)
	SetCaptureActive(false);

	WindowMID = UMaterialInstanceDynamic::Create(Material, this);
	if (!WindowMID)
	{
		return false;
	}
	WindowMID->SetTextureParameterValue(TromboneMaterial::OcclusionCaptureRTParam, CaptureRT);
	WindowMID->SetScalarParameterValue(TromboneMaterial::WindowFadeParam, 0.f);
	// 모양 파라미터는 변하지 않으므로 한 번만 밀어 넣는다
	WindowMID->SetScalarParameterValue(TromboneMaterial::EdgeSoftnessParam, EdgeSoftness);
	WindowMID->SetScalarParameterValue(TromboneMaterial::WallDimParam, WallDim);
	// 캡처가 화면 전체를 담는다는 뜻. 크롭 모드면 UpdateEffect가 매 프레임 실제 영역으로 덮어쓴다
	WindowMID->SetVectorParameterValue(TromboneMaterial::CaptureRectMinParam, FLinearColor(0.f, 0.f, 0.f, 0.f));
	WindowMID->SetVectorParameterValue(TromboneMaterial::CaptureRectSizeParam, FLinearColor(1.f, 1.f, 0.f, 0.f));

	// weight 0 = 패스 자체가 제거되어 비용 없음
	Cam->PostProcessSettings.WeightedBlendables.Array.Add(FWeightedBlendable(0.f, WindowMID));

	WindowFade = 0.f;
	return true;
}

void UXRayWindowComponent::OnTraceUpdated(const TArray<AActor*>& Occluders)
{
	CachedOccluders.Reset();
	CachedOccluders.Reserve(Occluders.Num());
	for (AActor* Occluder : Occluders)
	{
		CachedOccluders.Add(Occluder);
	}

	if (bCaptureActive)
	{
		ApplyHiddenActors();
	}
}

void UXRayWindowComponent::UpdateEffect(float DeltaTime)
{
	const AActor* Owner = GetOwner();
	UCameraComponent* Cam = GetCamera();
	if (!SceneCapture || !WindowMID || !Cam || !Owner)
	{
		return;
	}

	// 진행도는 등속으로 민다. 가속/감속은 아래 EaseOutBack이 전담해야 "확장" 곡선이 뭉개지지 않는다
	const bool bOccluding = IsAnyOccluding();
	WindowFade = FMath::FInterpConstantTo(
		WindowFade, bOccluding ? 1.f : 0.f, DeltaTime,
		1.f / FMath::Max(KINDA_SMALL_NUMBER, bOccluding ? OpenDuration : CloseDuration));

	if (!bCaptureActive)
	{
		if (WindowFade <= KINDA_SMALL_NUMBER)
		{
			return;
		}
		// 가림 시작: 숨길 액터를 즉시 반영하고 패스를 켠다 (트레이스 주기 0.1초를 기다리지 않는다)
		ApplyHiddenActors();
		SetCaptureActive(true);
		// weight는 0/1 이진으로만 쓴다. 0<weight<1이면 엔진이 MID의 모든 파라미터를
		// 베이스 머티리얼 기본값 쪽으로 보간해버려서 원 위치/반경이 흔들린다.
		SetBlendableWeight(1.f);
	}
	else if (!bOccluding && FMath::IsNearlyZero(WindowFade, 0.01f))
	{
		WindowFade = 0.f;
		WindowMID->SetScalarParameterValue(TromboneMaterial::WindowFadeParam, 0.f);
		SetCaptureActive(false);
		SetBlendableWeight(0.f);
		return;
	}

	const APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	// 실제로 화면을 그리는 FOV는 CameraManager가 들고 있다 (카메라 셰이크/줌이 여기 반영됨).
	// 이 값으로 캡처를 맞춰야 캡처 UV와 화면 UV가 어긋나지 않는다.
	const float ViewFOV = PC->PlayerCameraManager
		? PC->PlayerCameraManager->GetFOVAngle()
		: Cam->FieldOfView;

	int32 ViewX = 0, ViewY = 0;
	PC->GetViewportSize(ViewX, ViewY);
	if (ViewX <= 0 || ViewY <= 0)
	{
		return;
	}

	// 창 크기가 바뀌면 RT도 따라가야 캡처 UV가 화면 UV와 계속 일치한다.
	// 크롭 모드는 RT가 고정 크기고 매핑을 CaptureRect가 담당하므로 해당 없음
	if (!bCropCaptureToWindow && LastViewportSize != FIntPoint(ViewX, ViewY))
	{
		LastViewportSize = FIntPoint(ViewX, ViewY);
		CaptureRT->ResizeTarget(
			FMath::Max(1, FMath::RoundToInt(ViewX * CaptureResolutionScale)),
			FMath::Max(1, FMath::RoundToInt(ViewY * CaptureResolutionScale)));
	}

	const float ViewAspect = static_cast<float>(ViewX) / ViewY;

	// 메인 뷰는 AspectRatio_MaintainYFOV(엔진 기본값)로 투영을 만들기 때문에 유효 "수평" 반각이
	// 카메라의 AspectRatio에 따라 달라진다: atan(tan(FOV/2) * 뷰포트종횡비 / 카메라AspectRatio).
	// 반면 SceneCapture의 FOVAngle은 항상 수평 전각으로 해석되고 종횡비는 렌더타겟에서 가져온다.
	// 이 보정을 빼먹으면 뷰포트 종횡비가 카메라 AspectRatio(기본 1.7778)와 다를 때
	// 캡처가 더 좁은 화각을 담게 되어 원 안이 확대되어 보인다.
	const float CameraAspect = (Cam->AspectRatio > KINDA_SMALL_NUMBER) ? Cam->AspectRatio : ViewAspect;
	const float HalfFovTan = FMath::Tan(FMath::DegreesToRadians(ViewFOV * 0.5f)) * ViewAspect / CameraAspect;

	SceneCapture->FOVAngle = FMath::RadiansToDegrees(2.f * FMath::Atan(HalfFovTan));

	const FVector OwnerLocation = Owner->GetActorLocation();

	// 투영에 실패하면(캐릭터가 화면 밖) 원이 (0,0)으로 튀므로 이번 프레임은 이전 값 유지
	FVector2D ScreenPos = FVector2D::ZeroVector;
	if (!PC->ProjectWorldLocationToScreen(OwnerLocation, ScreenPos, /*bPlayerViewportRelative=*/true))
	{
		return;
	}

	const FVector2D CenterUV(ScreenPos.X / ViewX, ScreenPos.Y / ViewY);

	// xy=원 중심 UV, z=종횡비. 머티리얼이 C++과 같은 값으로 원을 보정하기 위함
	WindowMID->SetVectorParameterValue(
		TromboneMaterial::PlayerScreenUVParam,
		FLinearColor(CenterUV.X, CenterUV.Y, ViewAspect, 0.f));

	// 월드 반경(cm)을 화면 반경으로 투영. 위에서 구한 "메인 뷰의 실제 수평 반각"을 그대로 써야
	// 원 크기도 화면과 같은 비율로 맞는다 (보정 전에는 원도 같은 배율로 어긋나 있었다)
	const float Distance = FVector::Dist(Cam->GetComponentLocation(), OwnerLocation);
	const float RadiusPixels = (Distance > KINDA_SMALL_NUMBER && HalfFovTan > KINDA_SMALL_NUMBER)
		? WorldHoleRadius * (ViewX * 0.5f) / (Distance * HalfFovTan)
		: 0.f;

	// 반경 자체를 진행도로 스케일해야 원이 캐릭터 중앙(반경 0)에서 확장돼 나온다.
	// 열릴 때만 오버슈트를 주고, 닫힐 때는 그대로 수축시킨다
	const float RadiusUV = RadiusPixels / ViewY;
	const float RadiusAlpha = bOccluding ? EaseOutBack(WindowFade, PopOvershoot) : WindowFade;
	WindowMID->SetScalarParameterValue(
		TromboneMaterial::HoleScreenRadiusParam, RadiusUV * RadiusAlpha);

	// 내용물은 원이 커지는 것보다 훨씬 빨리 불투명해져야 "서서히 나타남"이 아니라 "구멍이 뚫림"으로 읽힌다.
	// 다만 (1 - OccluderOpacity)까지만 덮어서, 나머지는 원래 화면(=가리는 물체가 있는 화면)이
	// 비쳐 보이게 한다 → 천막이 통째로 사라지지 않고 반투명으로 남는다
	WindowMID->SetScalarParameterValue(
		TromboneMaterial::WindowFadeParam,
		FMath::Clamp(WindowFade * 3.f, 0.f, 1.f) * (1.f - OccluderOpacity));

	if (bCropCaptureToWindow)
	{
		UpdateCaptureCrop(PC, CenterUV, RadiusUV, ViewAspect);
	}
}

void UXRayWindowComponent::UpdateCaptureCrop(const APlayerController* PC, const FVector2D& CenterUV, float RadiusUV, float ViewAspect)
{
	FVector2D RectMin, RectSize;
	if (!ComputeCaptureRect(CenterUV, RadiusUV, ViewAspect, RectMin, RectSize))
	{
		return;
	}

	// 화면을 그리는 것과 똑같은 투영을 가져온다. ProjectWorldLocationToScreen도 같은 경로를 쓰므로
	// 위에서 구한 원 중심 UV와 이 행렬이 어긋날 수 없다
	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	FSceneViewProjectionData ProjectionData;
	if (!LocalPlayer || !LocalPlayer->ViewportClient ||
		!LocalPlayer->GetProjectionData(LocalPlayer->ViewportClient->Viewport, ProjectionData))
	{
		return;
	}

	// 행렬과 머티리얼 쪽 렉트는 반드시 같은 프레임에 같은 값으로 맞춰야 한다.
	// 하나만 갱신되면 원 안의 그림이 어긋난다
	SceneCapture->CustomProjectionMatrix = BuildCroppedProjection(ProjectionData.ProjectionMatrix, RectMin, RectSize);
	SceneCapture->bUseCustomProjectionMatrix = true;
	WindowMID->SetVectorParameterValue(
		TromboneMaterial::CaptureRectMinParam, FLinearColor(RectMin.X, RectMin.Y, 0.f, 0.f));
	WindowMID->SetVectorParameterValue(
		TromboneMaterial::CaptureRectSizeParam, FLinearColor(RectSize.X, RectSize.Y, 0.f, 0.f));

	// 렉트가 한 번에 크게 튀면(카메라 컷/텔레포트) 이전 프레임 기준으로 쌓인
	// 오클루전 컬링 히스토리가 맞지 않아 물체가 한 프레임 사라질 수 있다
	const FVector2D RectCenter = RectMin + RectSize * 0.5f;
	if (FVector2D::DistSquared(RectCenter, LastRectCenter) > FMath::Square(RectJumpThreshold))
	{
		SceneCapture->bCameraCutThisFrame = true;
	}
	LastRectCenter = RectCenter;
}

bool UXRayWindowComponent::ComputeCaptureRect(
	const FVector2D& CenterUV, float RadiusUV, float ViewAspect, FVector2D& OutMin, FVector2D& OutSize) const
{
	// 원은 열릴 때 EaseOutBack으로 최종 크기를 잠깐 넘어선다. 그 최댓값까지 미리 덮어둬야
	// 확장하는 순간에 사각형 경계가 드러나지 않는다
	const float PeakScale = 1.f + (4.f * FMath::Cube(PopOvershoot)) / (27.f * FMath::Square(PopOvershoot + 1.f));

	// 반경은 애니메이션이 적용되기 전의 최종 크기를 쓴다. 원이 커지는 동안 렉트가 같이 자라면
	// 캡처 배율이 매 프레임 변해 원 안이 일렁인다
	const double HalfV = RadiusUV * PeakScale * CropMarginFactor;
	const double HalfU = (ViewAspect > KINDA_SMALL_NUMBER) ? HalfV / ViewAspect : HalfV;

	// 화면 밖은 어차피 그려지지 않으므로 잘라낸다. 잘린 만큼 캡처 픽셀을 아낀다
	const FVector2D Min(
		FMath::Clamp(CenterUV.X - HalfU, 0.0, 1.0),
		FMath::Clamp(CenterUV.Y - HalfV, 0.0, 1.0));
	const FVector2D Max(
		FMath::Clamp(CenterUV.X + HalfU, 0.0, 1.0),
		FMath::Clamp(CenterUV.Y + HalfV, 0.0, 1.0));

	OutMin = Min;
	OutSize = Max - Min;

	// 원이 통째로 화면 밖이면 렉트가 0이 된다. 투영이 퇴화하므로 이번 프레임은 건드리지 않는다
	return OutSize.X > KINDA_SMALL_NUMBER && OutSize.Y > KINDA_SMALL_NUMBER;
}

FMatrix UXRayWindowComponent::BuildCroppedProjection(
	const FMatrix& CameraProjection, const FVector2D& RectMin, const FVector2D& RectSize)
{
	// UV를 NDC로. u=(x+1)/2 이므로 중심은 (u0+u1)-1, 반폭은 렉트 폭 그대로다.
	// v는 위아래가 뒤집혀 있어 부호가 반대
	const FVector2D RectMax = RectMin + RectSize;
	const double CenterX = RectMin.X + RectMax.X - 1.0;
	const double CenterY = 1.0 - (RectMin.Y + RectMax.Y);

	// 클립 공간에서 렉트를 화면 전체로 펴는 변환. z/w 행은 손대지 않아
	// 근평면과 reversed-Z가 카메라 투영에서 그대로 따라온다
	FMatrix Crop = FMatrix::Identity;
	Crop.M[0][0] = 1.0 / RectSize.X;
	Crop.M[1][1] = 1.0 / RectSize.Y;
	Crop.M[3][0] = -CenterX / RectSize.X;
	Crop.M[3][1] = -CenterY / RectSize.Y;

	return CameraProjection * Crop;
}

void UXRayWindowComponent::SetWindowShape(float InWorldHoleRadius, float InEdgeSoftness, float InOccluderOpacity)
{
	WorldHoleRadius = FMath::Max(0.f, InWorldHoleRadius);
	EdgeSoftness = FMath::Clamp(InEdgeSoftness, 0.f, 1.f);
	// 알파는 UpdateEffect가 매 프레임 다시 계산하므로 값만 넣어두면 다음 틱에 반영된다
	OccluderOpacity = FMath::Clamp(InOccluderOpacity, 0.f, 1.f);

	// 반경은 UpdateEffect가 매 프레임 카메라 거리로 다시 계산해 밀어넣지만,
	// EdgeSoftness는 초기화 때 한 번만 넣으므로 여기서 직접 갱신해야 즉시 반영된다
	if (WindowMID)
	{
		WindowMID->SetScalarParameterValue(TromboneMaterial::EdgeSoftnessParam, EdgeSoftness);
	}
}

void UXRayWindowComponent::ApplyHiddenActors()
{
	if (!SceneCapture)
	{
		return;
	}

	SceneCapture->HiddenActors.Reset();
	for (const TWeakObjectPtr<AActor>& Occluder : CachedOccluders)
	{
		if (AActor* Actor = Occluder.Get())
		{
			SceneCapture->HiddenActors.Add(Actor);
		}
	}

	RefreshOccluderStencils();
}

void UXRayWindowComponent::RefreshOccluderStencils()
{
	TSet<TWeakObjectPtr<AActor>> NewSet;
	NewSet.Reserve(CachedOccluders.Num());

	for (const TWeakObjectPtr<AActor>& Occluder : CachedOccluders)
	{
		AActor* Actor = Occluder.Get();
		if (!Actor)
		{
			continue;
		}

		TArray<UMeshComponent*> Meshes;
		Actor->GetComponents<UMeshComponent>(Meshes);
		for (UMeshComponent* Mesh : Meshes)
		{
			Mesh->SetCustomDepthStencilValue(TromboneRender::OCCLUDER_STENCIL);
			Mesh->SetRenderCustomDepth(true);
		}
		NewSet.Add(Actor);
	}

	// 더 이상 가리지 않는 액터는 CustomDepth 렌더를 꺼서 비용을 남기지 않는다
	for (const TWeakObjectPtr<AActor>& Old : StenciledOccluders)
	{
		if (Old.IsValid() && !NewSet.Contains(Old))
		{
			TArray<UMeshComponent*> Meshes;
			Old->GetComponents<UMeshComponent>(Meshes);
			for (UMeshComponent* Mesh : Meshes)
			{
				Mesh->SetRenderCustomDepth(false);
			}
		}
	}

	StenciledOccluders = MoveTemp(NewSet);
}

void UXRayWindowComponent::ClearOccluderStencils()
{
	for (const TWeakObjectPtr<AActor>& Occluder : StenciledOccluders)
	{
		if (!Occluder.IsValid())
		{
			continue;
		}

		TArray<UMeshComponent*> Meshes;
		Occluder->GetComponents<UMeshComponent>(Meshes);
		for (UMeshComponent* Mesh : Meshes)
		{
			Mesh->SetRenderCustomDepth(false);
		}
	}
	StenciledOccluders.Empty();
}

void UXRayWindowComponent::SetCaptureActive(bool bActive)
{
	if (!SceneCapture)
	{
		return;
	}

	bCaptureActive = bActive;
	SceneCapture->SetVisibility(bActive);
	SceneCapture->SetComponentTickEnabled(bActive);

	// 지난번 켜졌을 때와 시야가 전혀 다를 수 있다. 그때 쌓인 오클루전 히스토리를 물려받으면
	// 켜지는 첫 프레임에 물체가 비어 보인다
	if (bActive)
	{
		SceneCapture->bCameraCutThisFrame = true;
	}

	// 가려지지 않을 때는 CustomDepth 렌더도 같이 꺼둔다
	if (!bActive)
	{
		ClearOccluderStencils();
	}
}

void UXRayWindowComponent::SetBlendableWeight(float Weight)
{
	UCameraComponent* Cam = GetCamera();
	if (!Cam || !WindowMID)
	{
		return;
	}

	for (FWeightedBlendable& Blendable : Cam->PostProcessSettings.WeightedBlendables.Array)
	{
		if (Blendable.Object == WindowMID)
		{
			Blendable.Weight = Weight;
			return;
		}
	}
}

void UXRayWindowComponent::TeardownEffect()
{
	ClearOccluderStencils();

	if (UCameraComponent* Cam = GetCamera())
	{
		if (WindowMID)
		{
			Cam->PostProcessSettings.WeightedBlendables.Array.RemoveAll(
				[this](const FWeightedBlendable& Blendable) { return Blendable.Object == WindowMID; });
		}
	}

	if (SceneCapture)
	{
		SceneCapture->DestroyComponent();
	}

	SceneCapture = nullptr;
	CaptureRT = nullptr;
	WindowMID = nullptr;
	CachedOccluders.Reset();
	WindowFade = 0.f;
	bCaptureActive = false;
	LastViewportSize = FIntPoint::ZeroValue;
	LastRectCenter = FVector2D::ZeroVector;
}
