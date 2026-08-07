// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Components/ActorComponents/XRaySilhouetteComponent.h"
#include "Camera/CameraComponent.h"
#include "Characters/TromboneCharacterBase.h"
#include "Components/MeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Utilities/Defines.h"

void UXRaySilhouetteComponent::ApplyOccludedStencil(UPrimitiveComponent* Prim)
{
	if (!Prim)
	{
		return;
	}
	Prim->SetRenderCustomDepth(true);
	Prim->SetCustomDepthStencilValue(TromboneRender::CHARACTER_OCCLUDED_STENCIL);
}

void UXRaySilhouetteComponent::ClearOccludedStencil(UPrimitiveComponent* Prim)
{
	if (!Prim)
	{
		return;
	}
	Prim->SetRenderCustomDepth(false);
}

void UXRaySilhouetteComponent::ApplyStencilToActor(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}
	// 메시만 대상. 네임플레이트 위젯이나 콜리전 프리미티브까지 찍으면 실루엣에 섞여 나온다
	TArray<UMeshComponent*> Meshes;
	Actor->GetComponents<UMeshComponent>(Meshes);
	for (UMeshComponent* Mesh : Meshes)
	{
		ApplyOccludedStencil(Mesh);
	}
}

void UXRaySilhouetteComponent::ClearStencilFromActor(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}
	TArray<UMeshComponent*> Meshes;
	Actor->GetComponents<UMeshComponent>(Meshes);
	for (UMeshComponent* Mesh : Meshes)
	{
		ClearOccludedStencil(Mesh);
	}
}

bool UXRaySilhouetteComponent::InitializeEffect()
{
	UCameraComponent* Cam = GetCamera();
	if (!Cam)
	{
		return false;
	}

	UMaterialInterface* Material = SilhouetteMaterial;
	if (!Material)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[XRaySilhouette] 실루엣 머티리얼이 없습니다. BP의 XRay|Silhouette > SilhouetteMaterial 에 "
			     "M_XRaySilhouette 를 지정하세요."));
		return false;
	}

	SilhouetteMID = UMaterialInstanceDynamic::Create(Material, this);
	if (!SilhouetteMID)
	{
		return false;
	}

	// MID는 여기서 처음 생기므로, 그전에 온 피부색 통지는 흘렸다. 현재 값을 당겨와 메운다
	// (이후 변경은 OnSkinColorChanged 구독으로 들어온다)
	if (const ATromboneCharacterBase* Character = Cast<ATromboneCharacterBase>(GetOwner()))
	{
		SilhouetteMID->SetVectorParameterValue(TromboneMaterial::SilhouetteColorParam, Character->GetSkinColor());
	}

	// weight 0 = 패스 자체가 제거되어 비용 없음
	Cam->PostProcessSettings.WeightedBlendables.Array.Add(FWeightedBlendable(0.f, SilhouetteMID));
	bSilhouetteVisible = false;
	return true;
}

void UXRaySilhouetteComponent::UpdateEffect(float DeltaTime)
{
	// weight는 0/1 이진으로만 쓴다. 0<weight<1이면 엔진이 MID의 모든 파라미터를
	// 베이스 머티리얼 기본값 쪽으로 보간해버린다
	const bool bShouldShow = IsAnyOccluding();
	if (bShouldShow != bSilhouetteVisible)
	{
		bSilhouetteVisible = bShouldShow;
		SetBlendableWeight(bShouldShow ? 1.f : 0.f);
	}
}

void UXRaySilhouetteComponent::OnTraceUpdated(const TArray<AActor*>& Occluders)
{
	RefreshStencilTargets();
}

void UXRaySilhouetteComponent::RefreshStencilTargets()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// 커마 파츠(안테나/코스튬)는 오너에 붙는 메시 컴포넌트라 오너를 다시 훑으면 자동 포함되고,
	// 악기 본체/무기는 별도 액터로 어태치되므로 재귀 순회로 잡는다
	TArray<AActor*> Current;
	Current.Add(Owner);
	Owner->GetAttachedActors(Current, /*bResetArray=*/false, /*bRecursivelyIncludeAttachedActors=*/true);

	TSet<TWeakObjectPtr<AActor>> NewSet;
	NewSet.Reserve(Current.Num());
	for (AActor* Actor : Current)
	{
		if (!Actor)
		{
			continue;
		}
		// SetRenderCustomDepth는 값이 그대로면 아무 일도 안 하므로 매번 다시 찍어도 비용이 없다
		ApplyStencilToActor(Actor);
		NewSet.Add(Actor);
	}

	// 이번에 빠진 액터(무기 드롭 등)는 스텐실 해제
	for (const TWeakObjectPtr<AActor>& Old : StenciledActors)
	{
		if (Old.IsValid() && !NewSet.Contains(Old))
		{
			ClearStencilFromActor(Old.Get());
		}
	}

	StenciledActors = MoveTemp(NewSet);
}

void UXRaySilhouetteComponent::ClearAllStencils()
{
	for (const TWeakObjectPtr<AActor>& Actor : StenciledActors)
	{
		if (Actor.IsValid())
		{
			ClearStencilFromActor(Actor.Get());
		}
	}
	StenciledActors.Empty();
}

void UXRaySilhouetteComponent::OnSkinColorChanged(const FLinearColor& NewSkinColor)
{
	SetSilhouetteColor(NewSkinColor);
}

void UXRaySilhouetteComponent::SetSilhouetteColor(const FLinearColor& InColor)
{
	if (SilhouetteMID)
	{
		SilhouetteMID->SetVectorParameterValue(TromboneMaterial::SilhouetteColorParam, InColor);
	}
}

void UXRaySilhouetteComponent::SetBlendableWeight(float Weight)
{
	UCameraComponent* Cam = GetCamera();
	if (!Cam || !SilhouetteMID)
	{
		return;
	}

	for (FWeightedBlendable& Blendable : Cam->PostProcessSettings.WeightedBlendables.Array)
	{
		if (Blendable.Object == SilhouetteMID)
		{
			Blendable.Weight = Weight;
			return;
		}
	}
}

void UXRaySilhouetteComponent::TeardownEffect()
{
	ClearAllStencils();

	if (UCameraComponent* Cam = GetCamera())
	{
		if (SilhouetteMID)
		{
			Cam->PostProcessSettings.WeightedBlendables.Array.RemoveAll(
				[this](const FWeightedBlendable& Blendable) { return Blendable.Object == SilhouetteMID; });
		}
	}

	SilhouetteMID = nullptr;
	bSilhouetteVisible = false;
}
