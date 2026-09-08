// Copyright (C) 2026 biksari studio. All Rights Reserved.


#include "Actors/Gimmick/Drunkard/DrunkardDoorBreakerComponent.h"
#include "Actors/Gimmick/Breakable/BreakableDoor.h"
#include "Actors/Gimmick/Drunkard/DrunkardNPC.h"
#include "Actors/Gimmick/Drunkard/DrunkardSpawner.h"
#include "Data/DrunkardDataAsset.h"
#include "Utilities/TromboneLogs.h"

UDrunkardDoorBreakerComponent::UDrunkardDoorBreakerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDrunkardDoorBreakerComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority()) return;

	ADrunkardSpawner* Spawner = Cast<ADrunkardSpawner>(Owner);
	if (!Spawner)
	{
		UE_LOG(LogDrunkard, Warning, TEXT("[문 파괴] ADrunkardSpawner가 아닌 %s 에 붙어 있어 동작하지 않습니다"), *Owner->GetName());
		return;
	}

	Spawner->OnDrunkardSpawned.AddDynamic(this, &ThisClass::HandleDrunkardSpawned);
	ValidateBindings();
}

void UDrunkardDoorBreakerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BreakTimerHandle);
	}

	if (ADrunkardSpawner* Spawner = Cast<ADrunkardSpawner>(GetOwner()))
	{
		Spawner->OnDrunkardSpawned.RemoveDynamic(this, &ThisClass::HandleDrunkardSpawned);
	}

	Super::EndPlay(EndPlayReason);
}

void UDrunkardDoorBreakerComponent::ValidateBindings() const
{
	if (DoorBindings.IsEmpty())
	{
		UE_LOG(LogDrunkard, Warning, TEXT("[문 파괴] 바인딩이 비어 있습니다. 레벨의 스포너에서 스폰 지점과 문을 짝지어 주세요"));
		return;
	}

	for (int32 Index = 0; Index < DoorBindings.Num(); ++Index)
	{
		const FDrunkardDoorBinding& Binding = DoorBindings[Index];

		if (!Binding.SpawnPoint || !Binding.Door)
		{
			UE_LOG(LogDrunkard, Warning, TEXT("[문 파괴] 바인딩 %d 번이 비어 있습니다 (지점 %s / 문 %s)"),
				Index,
				Binding.SpawnPoint ? *Binding.SpawnPoint->GetName() : TEXT("없음"),
				Binding.Door ? *Binding.Door->GetName() : TEXT("없음"));
			continue;
		}

		const double Distance = FVector::Dist(Binding.SpawnPoint->GetActorLocation(), Binding.Door->GetActorLocation());
		if (Distance > BindingWarnDistance)
		{
			UE_LOG(LogDrunkard, Warning, TEXT("[문 파괴] 바인딩 %d 번의 지점 %s 와 문 %s 가 %.0fcm 떨어져 있습니다. 짝을 잘못 지은 게 아닌지 확인하세요"),
				Index, *Binding.SpawnPoint->GetName(), *Binding.Door->GetName(), Distance);
		}
	}
}

const FDrunkardDoorBinding* UDrunkardDoorBreakerComponent::FindBinding(const AActor* SpawnPoint) const
{
	if (!SpawnPoint) return nullptr;

	return DoorBindings.FindByPredicate(
		[SpawnPoint](const FDrunkardDoorBinding& Binding) { return Binding.SpawnPoint == SpawnPoint; });
}

void UDrunkardDoorBreakerComponent::HandleDrunkardSpawned(ADrunkardNPC* NPC, AActor* SpawnPoint)
{
	if (!NPC || !SpawnPoint) return;

	UWorld* World = GetWorld();
	if (!World) return;

	const FDrunkardDoorBinding* Binding = FindBinding(SpawnPoint);
	if (!Binding)
	{
		UE_LOG(LogDrunkard, Warning, TEXT("[문 파괴] 스폰 지점 %s 에 바인딩된 문이 없습니다"), *SpawnPoint->GetName());
		return;
	}

	ABreakableDoor* Door = Binding->Door;
	// 한 판 동안 부서진 채 유지되므로 같은 지점의 두 번째 스폰에서는 할 일이 없다
	if (!Door || IBreakable::Execute_IsBroken(Door)) return;

	// 취객 캡슐이 문 평면을 지나는 시점. 문 뒤에서 EnterBurstDuration 동안 실내로 이동한다
	float Delay = BreakDelaySeconds;
	if (Delay < 0.f)
	{
		const UDrunkardDataAsset* Data = NPC->GetDrunkardData();
		Delay = 0.5f * (Data ? Data->EnterBurstDuration : 0.5f);
	}

	FVector DoorOrigin, DoorExtent;
	Door->GetActorBounds(/*bOnlyCollidingComponents*/ false, DoorOrigin, DoorExtent);

	FBreakHitInfo Info;
	Info.Source = EBreakSource::Script;
	Info.ImpactPoint = DoorOrigin;
	// 스폰 지점의 forward = 실내 방향. 파편이 취객 진행 방향으로 날아간다
	Info.ImpactDirection = SpawnPoint->GetActorForwardVector().GetSafeNormal();
	Info.Strength = BreakStrength;
	Info.Instigator = NPC;

	World->GetTimerManager().SetTimer(
		BreakTimerHandle,
		FTimerDelegate::CreateUObject(this, &ThisClass::BreakDoor, TWeakObjectPtr<ABreakableDoor>(Door), Info),
		Delay,
		false);
}

void UDrunkardDoorBreakerComponent::BreakDoor(TWeakObjectPtr<ABreakableDoor> Door, FBreakHitInfo HitInfo)
{
	ABreakableDoor* TargetDoor = Door.Get();
	if (!TargetDoor || IBreakable::Execute_IsBroken(TargetDoor)) return;

	if (IBreakable::Execute_Break(TargetDoor, HitInfo))
	{
		UE_LOG(LogDrunkard, Log, TEXT("[문 파괴] %s 파괴"), *TargetDoor->GetName());
	}
}
