// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Gimmick/PuddleTrap/WaterDrop.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Actors/Gimmick/PuddleTrap/PuddleTrap.h"
#include "Kismet/KismetMathLibrary.h"

AWaterDrop::AWaterDrop()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);
	
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	if (CollisionComponent)
	{
		SetRootComponent(CollisionComponent);
		CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
		// "땅" 을 WorldStatic으로 쓴다고 가정하고, 이것만 Block
		CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

		CollisionComponent->SetSimulatePhysics(true);
		CollisionComponent->SetEnableGravity(true);
		CollisionComponent->CanCharacterStepUpOn = ECB_No;

		// Hit 이벤트 받기
		CollisionComponent->SetNotifyRigidBodyCollision(true);
	}

	// 시각용 메쉬
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	if (MeshComponent)
	{
		MeshComponent->SetupAttachment(CollisionComponent);
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeshComponent->CanCharacterStepUpOn = ECB_No;
	}
}

void AWaterDrop::BeginPlay()
{
	Super::BeginPlay();
	if (CollisionComponent)
	{
		CollisionComponent->OnComponentHit.AddDynamic(this, &ThisClass::OnCollisionHit);
	}
}

void AWaterDrop::OnCollisionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (bHasSpawnedPuddle || !HasAuthority())
	{
		return;
	}

	// 자기 자신과의 충돌이면 무시
	if (!OtherActor || OtherActor == this || !OtherComp)
	{
		return;
	}

	// "땅"이랑만 충돌하게 하고 싶으니, WorldStatic만 체크
	if (OtherComp->GetCollisionObjectType() != ECC_WorldStatic)
	{
		return;
	}

	bHasSpawnedPuddle = true;

	if (PuddleTrapClass)
	{
		// ImpactPoint를 기준으로 약간 위로 올려서 스폰
		const FVector SpawnLocation = Hit.ImpactPoint + Hit.ImpactNormal * 2.f;

		// 웅덩이의 Up 방향을 바닥 노멀에 맞춤
		const FRotator SpawnRotation =
			UKismetMathLibrary::MakeRotFromZ(Hit.ImpactNormal);

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		GetWorld()->SpawnActor<APuddleTrap>(
			PuddleTrapClass,
			FTransform(SpawnRotation, SpawnLocation),
			Params);
	}

	// 물방울은 역할 끝났으니 제거
	Destroy();
}
