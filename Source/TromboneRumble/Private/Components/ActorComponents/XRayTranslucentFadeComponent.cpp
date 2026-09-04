// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Components/ActorComponents/XRayTranslucentFadeComponent.h"
#include "Actors/XRayFadeMaterialProvider.h"
#include "Components/MeshComponent.h"
#include "Data/XRayFadeMaterialMap.h"
#include "LocalVertexFactory.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "PSOPrecache.h"
#include "Utilities/DebugHelper.h"
#include "Utilities/Defines.h"

bool UXRayTranslucentFadeComponent::InitializeEffect()
{
	// 로딩이 아니라 이미 레벨과 함께 올라온 객체의 포인터를 집는 것이라 비용이 없다
	FadeMap = AXRayFadeMaterialProvider::FindMapInLevel(this);

	if (!FadeMap || FadeMap->FadeVariants.IsEmpty())
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("페이드 머티리얼 맵이 없어 디더 페이드를 건너뜁니다. 레벨에 AXRayFadeMaterialProvider를 배치하고 맵을 지정하세요."));
		return false;
	}

	// 첫 교체 때 셰이더 파이프라인이 없으면 히칭이 나므로 미리 데운다
	TArray<UMaterialInterface*> Variants;
	FadeMap->GetAllFadeVariants(Variants);

	const FVertexFactoryType* VertexFactoryTypes[] = { &FLocalVertexFactory::StaticType };
	const FPSOPrecacheParams PrecacheParams;
	for (UMaterialInterface* Variant : Variants)
	{
		Variant->PrecachePSOs(MakeArrayView(VertexFactoryTypes), PrecacheParams);
	}

	return true;
}

void UXRayTranslucentFadeComponent::OnTraceUpdated(const TArray<AActor*>& InOccluders)
{
	for (TPair<TWeakObjectPtr<AActor>, FOccluderState>& Pair : Occluders)
	{
		Pair.Value.bOccluding = false;
	}

	for (AActor* HitActor : InOccluders)
	{
		FOccluderState& State = Occluders.FindOrAdd(HitActor);
		if (!State.bSlotsInitialized)
		{
			EnsureFadeSlots(HitActor, State);
			State.bSlotsInitialized = true;
		}
		State.bOccluding = true;
	}
}

void UXRayTranslucentFadeComponent::EnsureFadeSlots(const AActor* Actor, FOccluderState& State)
{
	TArray<UMeshComponent*> MeshComponents;
	Actor->GetComponents<UMeshComponent>(MeshComponents);

	for (UMeshComponent* MeshComp : MeshComponents)
	{
		const int32 NumMaterials = MeshComp->GetNumMaterials();
		for (int32 SlotIndex = 0; SlotIndex < NumMaterials; ++SlotIndex)
		{
			UMaterialInterface* Original = MeshComp->GetMaterial(SlotIndex);

			// 사본이 없는 머티리얼은 페이드 대상 아님 (스크립트를 안 돌린 에셋)
			UMaterialInterface* Variant = FadeMap->FindFadeVariant(Original);
			if (!Variant)
			{
				continue;
			}

			UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Variant, this);
			if (!MID)
			{
				continue;
			}

			MID->SetScalarParameterValue(TromboneMaterial::FadedOpacityParam, FadedOpacity);
			MID->SetScalarParameterValue(TromboneMaterial::OcclusionFadeParam, 0.f);
			FadeMIDs.Add(MID);

			FFadeSlot& Slot = State.Slots.AddDefaulted_GetRef();
			Slot.Mesh = MeshComp;
			Slot.SlotIndex = SlotIndex;
			Slot.Original = Original;
			Slot.FadeMID = MID;
		}
	}
}

void UXRayTranslucentFadeComponent::BindFadeMaterials(FOccluderState& State)
{
	for (const FFadeSlot& Slot : State.Slots)
	{
		if (Slot.Mesh.IsValid() && Slot.FadeMID.IsValid())
		{
			Slot.Mesh->SetMaterial(Slot.SlotIndex, Slot.FadeMID.Get());
		}
	}
	State.bBound = true;
}

void UXRayTranslucentFadeComponent::RestoreOriginalMaterials(FOccluderState& State)
{
	for (const FFadeSlot& Slot : State.Slots)
	{
		if (Slot.Mesh.IsValid() && Slot.Original.IsValid())
		{
			Slot.Mesh->SetMaterial(Slot.SlotIndex, Slot.Original.Get());
		}
	}
	State.bBound = false;
}

void UXRayTranslucentFadeComponent::UpdateEffect(float DeltaTime)
{
	for (auto It = Occluders.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid())
		{
			It.RemoveCurrent();
			continue;
		}

		FOccluderState& State = It.Value();

		// 가려지지도 않고 이미 원본으로 돌아간 항목은 매 프레임 건드릴 필요 없다.
		// 상태는 남겨둬서 다시 가려질 때 MID를 새로 만들지 않게 한다
		if (!State.bOccluding && !State.bBound)
		{
			continue;
		}

		const float TargetFade = State.bOccluding ? 1.f : 0.f;
		State.CurrentFade = FMath::FInterpTo(State.CurrentFade, TargetFade, DeltaTime, FadeInterpSpeed);

		const bool bFadedOut = !State.bOccluding && FMath::IsNearlyZero(State.CurrentFade, 0.01f);
		if (bFadedOut)
		{
			State.CurrentFade = 0.f;
		}

		for (const FFadeSlot& Slot : State.Slots)
		{
			if (Slot.FadeMID.IsValid())
			{
				Slot.FadeMID->SetScalarParameterValue(TromboneMaterial::OcclusionFadeParam, State.CurrentFade);
			}
		}

		// 파라미터를 먼저 맞춘 뒤 물려야 첫 프레임에 완전 투명으로 번쩍이지 않는다
		if (State.bOccluding && !State.bBound)
		{
			BindFadeMaterials(State);
		}
		else if (bFadedOut)
		{
			RestoreOriginalMaterials(State);
		}
	}
}

void UXRayTranslucentFadeComponent::TeardownEffect()
{
	// 파라미터만 0으로 두면 가림물이 사본을 문 채로 남는다. 반드시 원본을 되돌려야 한다
	for (TPair<TWeakObjectPtr<AActor>, FOccluderState>& Pair : Occluders)
	{
		if (Pair.Value.bBound)
		{
			RestoreOriginalMaterials(Pair.Value);
		}
	}

	Occluders.Empty();
	FadeMIDs.Empty();
	FadeMap = nullptr;
}
