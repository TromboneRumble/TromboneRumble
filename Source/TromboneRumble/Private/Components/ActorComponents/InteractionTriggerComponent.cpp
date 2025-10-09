// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ActorComponents/InteractionTriggerComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/ActorComponents/InteractorComponent.h"
#include "Utilities/DebugHelper.h"

UInteractionTriggerComponent::UInteractionTriggerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	TriggerVolume = CreateDefaultSubobject<USphereComponent>(TEXT("InteractTrigger"));
	TriggerVolume->InitSphereRadius(120.f);
	TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerVolume->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerVolume->SetGenerateOverlapEvents(true);
	TriggerVolume->SetSimulatePhysics(false);
}

void UInteractionTriggerComponent::Server_TryInteract_Implementation(AActor* InstigatorActor)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !bTriggerActive) return;

	// 간단한 재진입/경쟁 방지 락
	static const FName GateTag(TEXT("InteractionGateLocked"));
	if (GetOwner()->Tags.Contains(GateTag)) return;
	GetOwner()->Tags.Add(GateTag);

	AActor* TargetActor = GetOwner();
	if (TargetActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
	{
		const bool bCan = IInteractable::Execute_CanInteract(TargetActor, InstigatorActor);
		if (bCan)
		{
			IInteractable::Execute_Interact(TargetActor, InstigatorActor);
		}
	}

	// 락 해제 (소비했든 안 했든)
	GetOwner()->Tags.Remove(GateTag);
}


void UInteractionTriggerComponent::SetTriggerActive_Implementation(bool bActivate)
{
	if (!GetOwner() || bTriggerActive == bActivate) return; // 상태 변화 없음

	bTriggerActive = bActivate;
	//Trigger상태에 따른 충돌 설정
	OnRep_TriggerActive();

	ForceRemoveThisFromAllInteractors();
}

void UInteractionTriggerComponent::BeginPlay()
{
	Super::BeginPlay();
	AActor* Owner = GetOwner();
	USceneComponent* RootComp = Owner->GetRootComponent();
	if (TriggerVolume)
	{
		TriggerVolume->AttachToComponent(RootComp, FAttachmentTransformRules::KeepRelativeTransform);
		TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &UInteractionTriggerComponent::HandleBeginOverlap);
		TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &UInteractionTriggerComponent::HandleEndOverlap);
		SetCollisionEnabled(bTriggerActive);
	}

}

void UInteractionTriggerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInteractionTriggerComponent, bTriggerActive);
}

void UInteractionTriggerComponent::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bTriggerActive || !OtherActor)
	{
		Debug::Print(TEXT("TriggerNotActive or OtherActor is None"));
		return;
	}

	if (UInteractorComponent* Interactor = OtherActor->FindComponentByClass<UInteractorComponent>())
	{
		OverlappingInteractors.Add(Interactor);
		Interactor->RegisterCandidate(GetOwner());
		Debug::Print(FString::Printf(TEXT("Registered Candidate: %s"), *Interactor->GetOwner()->GetName()));
	}
	else
	{
		Debug::Print(TEXT("No UInteractorComponent Found"));
	}
}

void UInteractionTriggerComponent::HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor) return;

	if (UInteractorComponent* Interactor = OtherActor->FindComponentByClass<UInteractorComponent>())
	{
		OverlappingInteractors.Remove(Interactor);
		Interactor->UnregisterCandidate(GetOwner());
		Debug::Print(FString::Printf(TEXT("UnRegistered Candidate: %s"), *Interactor->GetOwner()->GetName()));
	}
}

void UInteractionTriggerComponent::OnRep_TriggerActive()
{
	SetCollisionEnabled(bTriggerActive);
}

void UInteractionTriggerComponent::SetupCollision(UShapeComponent* Shape)
{
	Shape->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Shape->SetCollisionResponseToAllChannels(ECR_Ignore);
	//TODO : 커스텀 채널 만들기
	Shape->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Shape->SetGenerateOverlapEvents(true);
}

void UInteractionTriggerComponent::SetCollisionEnabled(bool bEnable)
{
	if (!TriggerVolume) return;
	TriggerVolume->SetCollisionEnabled(bEnable ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	TriggerVolume->SetGenerateOverlapEvents(bEnable);
}

void UInteractionTriggerComponent::ForceRemoveThisFromAllInteractors()
{
	for (auto It = OverlappingInteractors.CreateIterator(); It; ++It)
	{
		if (UInteractorComponent* Interactor = It->Get())
		{
			Interactor->UnregisterCandidate(GetOwner());
		}
	}
	OverlappingInteractors.Empty();
}
