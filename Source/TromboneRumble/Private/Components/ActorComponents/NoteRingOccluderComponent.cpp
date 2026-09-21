// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Components/ActorComponents/NoteRingOccluderComponent.h"
#include "Components/SkinnedMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"
#include "Materials/MaterialInterface.h"
#include "MaterialShared.h"
#include "Subsystems/RhythmSubsystem.h"
#include "Utilities/Defines.h"

// 0이면 스텐실을 전부 해제한다 (링이 다시 캐릭터 위로 그려짐). 원인 확인·비용 측정용 A/B 스위치
static TAutoConsoleVariable<int32> CVarNoteRingOccluder(
	TEXT("Trombone.NoteRing.Occluder"), 1,
	TEXT("1이면 로컬 캐릭터가 발밑 노트 링을 가린다 (CustomStencil 252). 0이면 끈다."));

UNoteRingOccluderComponent::UNoteRingOccluderComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickInterval = 0.2f;
}

void UNoteRingOccluderComponent::BeginPlay()
{
	Super::BeginPlay();

	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		// 클라이언트는 BeginPlay 시점에 아직 조종권이 없을 수 있다
		OwnerPawn->ReceiveControllerChangedDelegate.AddDynamic(this, &ThisClass::HandleControllerChanged);
	}

	RefreshTickEnabled();
}

void UNoteRingOccluderComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		OwnerPawn->ReceiveControllerChangedDelegate.RemoveDynamic(this, &ThisClass::HandleControllerChanged);
	}

	ClearAllStencils();
	Super::EndPlay(EndPlayReason);
}

void UNoteRingOccluderComponent::HandleControllerChanged(APawn* InPawn, AController* OldController, AController* NewController)
{
	RefreshTickEnabled();
}

void UNoteRingOccluderComponent::RefreshTickEnabled()
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	const bool bLocal = OwnerPawn && OwnerPawn->IsLocallyControlled() && GetNetMode() != NM_DedicatedServer;

	SetComponentTickEnabled(bLocal);
	if (bLocal)
	{
		RefreshStencilTargets();
	}
	else
	{
		// 조종권을 잃으면 남의 화면 기준 표식이 남지 않게 지운다
		ClearAllStencils();
	}
}

void UNoteRingOccluderComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	RefreshStencilTargets();
}

void UNoteRingOccluderComponent::RefreshStencilTargets()
{
	AActor* Owner = GetOwner();
	const UWorld* World = GetWorld();
	const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	const URhythmSubsystem* RhythmSubsystem = GameInstance ? GameInstance->GetSubsystem<URhythmSubsystem>() : nullptr;

	// 링이 없는 레벨(로비 등)에서는 CustomDepth 비용을 내지 않는다
	const bool bActive = Owner && CVarNoteRingOccluder.GetValueOnGameThread() != 0
		&& RhythmSubsystem && RhythmSubsystem->GetRegisteredRhythmActor(World) != nullptr;
	if (!bActive)
	{
		ClearAllStencils();
		return;
	}

	// 악기/무기는 별도 액터로 어태치되므로 재귀 순회로 잡는다. 커마 파츠는 오너의 컴포넌트라 자동 포함
	TArray<AActor*> Actors;
	Actors.Add(Owner);
	Owner->GetAttachedActors(Actors, /*bResetArray=*/false, /*bRecursivelyIncludeAttachedActors=*/true);

	TSet<TWeakObjectPtr<UMeshComponent>> NewSet;
	TArray<UMeshComponent*> Meshes;
	for (const AActor* Actor : Actors)
	{
		if (!Actor)
		{
			continue;
		}
		Actor->GetComponents<UMeshComponent>(Meshes);
		for (UMeshComponent* Mesh : Meshes)
		{
			if (!IsOccluderMesh(Mesh))
			{
				continue;
			}
			// 값이 그대로면 두 세터 모두 아무 일도 안 하므로 매번 다시 찍어도 비용이 없다
			Mesh->SetRenderCustomDepth(true);
			Mesh->SetCustomDepthStencilValue(TromboneRender::CHARACTER_OCCLUDED_STENCIL);
			NewSet.Add(Mesh);
		}
	}

	// 이번에 빠진 메시(악기 드롭, 파츠 교체, 뒤늦게 반투명이 된 메시)는 해제
	for (const TWeakObjectPtr<UMeshComponent>& Old : StenciledMeshes)
	{
		if (Old.IsValid() && !NewSet.Contains(Old))
		{
			Old->SetRenderCustomDepth(false);
		}
	}
	StenciledMeshes = MoveTemp(NewSet);
}

void UNoteRingOccluderComponent::ClearAllStencils()
{
	for (const TWeakObjectPtr<UMeshComponent>& Mesh : StenciledMeshes)
	{
		if (Mesh.IsValid())
		{
			Mesh->SetRenderCustomDepth(false);
		}
	}
	StenciledMeshes.Empty();
}

bool UNoteRingOccluderComponent::IsOccluderMesh(const UMeshComponent* Mesh)
{
	// 위젯 컴포넌트도 UMeshComponent라 걸러야 한다. 안 거르면 콤보 글자 모양으로 링에 구멍이 난다
	if (!Mesh || !(Mesh->IsA<UStaticMeshComponent>() || Mesh->IsA<USkinnedMeshComponent>()))
	{
		return false;
	}

	// 링 메시(과녁, 거기 붙는 노트 링)는 반투명이라 여기서 빠진다
	for (int32 Index = 0; Index < Mesh->GetNumMaterials(); ++Index)
	{
		const UMaterialInterface* Material = Mesh->GetMaterial(Index);
		if (Material && !IsTranslucentBlendMode(*Material))
		{
			return true;
		}
	}
	return false;
}
