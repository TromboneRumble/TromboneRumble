// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ActorComponents/InteractionTriggerComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/ActorComponents/InteractorComponent.h"

UInteractionTriggerComponent::UInteractionTriggerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UInteractionTriggerComponent::Server_TryInteractAndConsume_Implementation(AActor* InstigatorActor)
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
			// 성공으로 간주하고 Trigger비활성화
			ActivateTrigger(false);
		}
	}
	

	// 락 해제 (소비했든 안 했든)
	GetOwner()->Tags.Remove(GateTag);
}


void UInteractionTriggerComponent::OnDroppedToWorld_Implementation()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	bTriggerActive = true;
	OnRep_TriggerActive();
	ForceRemoveThisFromAllInteractors();
}

void UInteractionTriggerComponent::BeginPlay()
{
	Super::BeginPlay();
	if (Trigger)
	{
		Trigger->OnComponentBeginOverlap.AddDynamic(this, &UInteractionTriggerComponent::HandleBeginOverlap);
		Trigger->OnComponentEndOverlap.AddDynamic(this, &UInteractionTriggerComponent::HandleEndOverlap);
		SetCollisionEnabled(bTriggerActive);
	}

}
void UInteractionTriggerComponent::OnRegister()
{
	Super::OnRegister();
	// Trigger 컴포넌트가 없으면 임의로 생성
	if (!Trigger && TriggerClass)
	{
		Trigger = NewObject<UShapeComponent>(GetOwner(), TriggerClass, TEXT("InteractTrigger"));
		Trigger->RegisterComponent();
		Trigger->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		SetupCollision(Trigger);
	}
	else if (!Trigger)
	{
		USphereComponent* Sphere = NewObject<USphereComponent>(GetOwner(), TEXT("InteractSphere"));
		Sphere->InitSphereRadius(120.f);
		Sphere->RegisterComponent();
		Sphere->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		Trigger = Sphere;
		SetupCollision(Trigger);
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
	if (!bTriggerActive || !OtherActor) return;

	if (UInteractorComponent* Interactor = OtherActor->FindComponentByClass<UInteractorComponent>())
	{
		OverlappingInteractors.Add(Interactor);
		Interactor->RegisterCandidate(GetOwner());
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
	}
}


void UInteractionTriggerComponent::ActivateTrigger(bool bActivate)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return; //서버에서만 작동
	if (bTriggerActive == bActivate) return; // 상태 변화 없음

	bTriggerActive = bActivate;
	OnRep_TriggerActive();

	if (!bTriggerActive)
	{
		// 즉시 모든 Interactor 후보에서 제거
		ForceRemoveThisFromAllInteractors();
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
	if (!Trigger) return;
	Trigger->SetCollisionEnabled(bEnable ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	Trigger->SetGenerateOverlapEvents(bEnable);
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
