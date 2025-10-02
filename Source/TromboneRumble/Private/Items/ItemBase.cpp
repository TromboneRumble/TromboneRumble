// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/ItemBase.h"
#include "Components/SphereComponent.h"
#include "Characters/TromboneCharacterBase.h"

AItemBase::AItemBase()
{
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>("ItemMeshComponent");
	ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetRootComponent(ItemMesh);

	BoundingSphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	BoundingSphere->SetupAttachment(ItemMesh);
}

void AItemBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AItemBase::Activate()
{
}

void AItemBase::BeginPlay()
{
	Super::BeginPlay();
	BoundingSphere->OnComponentBeginOverlap.AddDynamic(this, &AItemBase::OnSphereBeginOverlap);
	BoundingSphere->OnComponentEndOverlap.AddDynamic(this, &AItemBase::OnSphereEndOverlap);
}

void AItemBase::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

}

void AItemBase::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}



