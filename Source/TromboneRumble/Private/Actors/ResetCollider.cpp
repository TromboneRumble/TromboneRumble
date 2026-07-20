// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/ResetCollider.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Items/ItemBase.h"
#include "Kismet/GameplayStatics.h"
#include "Utilities/DebugHelper.h"

AResetCollider::AResetCollider()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	SetRootComponent(BoxComponent);
	BoxComponent->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	BoxComponent->SetCollisionProfileName(TEXT("Trigger"));
}

void AResetCollider::BeginPlay()
{
	Super::BeginPlay();

	// 서버에서만 바인딩하므로 overlap 텔레포트는 클라이언트에서 실행되지 않는다
	if (HasAuthority() && BoxComponent)
	{
		BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnBoxBeginOverlap);
	}
}

void AResetCollider::OnBoxBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!HasAuthority() || !OtherActor)
	{
		return;
	}

	TeleportActorToSpawn(OtherActor);
}

void AResetCollider::HandleServerRPC(ACharacter* InstigatorCharacter)
{
	// UI 비상탈출 요청. relay가 서버에서 호출하므로 여기는 이미 서버다.
	// 권한/유효성 검사는 TeleportActorToSpawn이 담당한다.
	TeleportActorToSpawn(InstigatorCharacter);
}

void AResetCollider::TeleportActorToSpawn(AActor* TargetActor)
{
	if (!HasAuthority() || !TargetActor)
	{
		return;
	}

	if (ACharacter* Character = Cast<ACharacter>(TargetActor))
	{
		Character->SetActorLocation(SpawnLocation, false, nullptr, ETeleportType::TeleportPhysics);
		return;
	}

	if (AItemBase* Item = Cast<AItemBase>(TargetActor))
	{
		if (Item->GetCurrentOwner())
		{
			return;
		}

		USkeletalMeshComponent* Mesh = Item->GetSkeletalMeshComponent();
		if (!Mesh)
		{
			return;
		}

		// 텔레포트 시 물리 충돌 및 속도 처리 (낙하 속도가 남으면 다시 떨어져 텔레포트가 반복됨)
		Mesh->SetSimulatePhysics(false);
		Item->SetActorLocation(SpawnLocation);
		Mesh->SetSimulatePhysics(true);

		Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
		Mesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}
}

AResetCollider* AResetCollider::FindInLevel(const UObject* WorldContextObject)
{
	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(WorldContextObject, StaticClass(), Found);

	if (Found.Num() == 0)
	{
		return nullptr;
	}

	if (Found.Num() > 1)
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, FString::Printf(TEXT("Multiple ResetColliders found (%d), using first"), Found.Num()));
	}

	return Cast<AResetCollider>(Found[0]);
}
