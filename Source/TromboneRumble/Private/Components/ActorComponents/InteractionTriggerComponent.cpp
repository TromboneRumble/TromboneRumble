// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/InteractionTriggerComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/ActorComponents/InteractorComponent.h"
#include "Interfaces/Interactable.h"
#include "Utilities/DebugHelper.h"

UInteractionTriggerComponent::UInteractionTriggerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	TriggerVolume = CreateDefaultSubobject<USphereComponent>(TEXT("InteractTrigger"));
	TriggerVolume->InitSphereRadius(120.f);
	TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerVolume->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerVolume->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
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
	bool bSuccess = false;
	
	if (TargetActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
	{
		bSuccess = IInteractable::Execute_CanInteract(TargetActor, InstigatorActor);
		if (bSuccess)
		{
			IInteractable::Execute_Interact(TargetActor, InstigatorActor);
		}
	}

	// 락 해제 (소비했든 안 했든)
	GetOwner()->Tags.Remove(GateTag);

	if (bSuccess)
	{
		if (UInteractorComponent* InstigatorInteractor = InstigatorActor->FindComponentByClass<UInteractorComponent>())
		{
			InstigatorInteractor->Client_OnInteractSuccess(TargetActor);
		}
	}
}

void UInteractionTriggerComponent::SetTriggerActive_Implementation(const bool bActivate)
{
	if (!GetOwner() || bTriggerActive == bActivate) return;

	bTriggerActive = bActivate;
	
	OnRep_TriggerActive();
	ForceRemoveThisFromAllInteractors();
}

void UInteractionTriggerComponent::BeginPlay()
{
	Super::BeginPlay();

	USceneComponent* RootComp = GetOwner()->GetRootComponent();
	if (TriggerVolume)
	{
		TriggerVolume->AttachToComponent(RootComp, FAttachmentTransformRules::KeepRelativeTransform);
		TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &UInteractionTriggerComponent::HandleBeginOverlap);
		TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &UInteractionTriggerComponent::HandleEndOverlap);
		SetCollisionEnabled(bTriggerActive);

		//스폰 시작시 이미 겹쳐있는 액터 처리
		if (bTriggerActive)
		{
			TArray<AActor*> OverlappingActors;
			TriggerVolume->GetOverlappingActors(OverlappingActors);

			for (AActor* OtherActor : OverlappingActors)
			{
				ProcessOverlap(OtherActor);
			}
		}
	}
}

void UInteractionTriggerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInteractionTriggerComponent, bTriggerActive);
}

void UInteractionTriggerComponent::ProcessOverlap(AActor* OtherActor)
{
	if (!OtherActor) return;

	if (UInteractorComponent* Interactor = OtherActor->FindComponentByClass<UInteractorComponent>())
	{
		OverlappingInteractors.Add(Interactor);
		Interactor->RegisterCandidate(GetOwner());
	}
}


void UInteractionTriggerComponent::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bTriggerActive) return;

	ProcessOverlap(OtherActor);
}

void UInteractionTriggerComponent::HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor) return;

	if (UInteractorComponent* Interactor = OtherActor->FindComponentByClass<UInteractorComponent>())
	{
		OverlappingInteractors.Remove(Interactor);
		Interactor->UnregisterCandidate(GetOwner());
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