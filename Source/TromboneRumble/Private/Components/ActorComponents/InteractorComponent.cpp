// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ActorComponents/InteractorComponent.h"

#include "Components/ActorComponents/InteractionTriggerComponent.h"
#include "Interfaces/Interactable.h"

UInteractorComponent::UInteractorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UInteractorComponent::TryInteract(AActor* ExplicitTarget)
{
    AActor* Target = ExplicitTarget ? ExplicitTarget : GetBestCandidate();
    if (!Target) return;

    Server_TryInteract(Target);
}

void UInteractorComponent::RegisterCandidate(AActor* Candidate)
{
    if (Candidate) Candidates.AddUnique(Candidate);
}

void UInteractorComponent::UnregisterCandidate(AActor* Candidate)
{
    Candidates.Remove(Candidate);
}

AActor* UInteractorComponent::GetBestCandidate() const
{
    // 마지막으로 들어온 후보를 최우선
    if (bPreferLastEntered)
    {
        for (int32 i = Candidates.Num() - 1; i >= 0; --i)
        {
            if (Candidates[i].IsValid()) return Candidates[i].Get();
        }
        return nullptr;
    }
    //TODO : Interact가능한 개체중 Trumpet이 있는지 체크
    return nullptr;
}

void UInteractorComponent::Server_TryInteract_Implementation(AActor* Target)
{
    if (!Target) return;
    if (UInteractionTriggerComponent* Trigger = Target->FindComponentByClass<UInteractionTriggerComponent>())
    {
        Trigger->Server_TryInteractAndConsume(GetOwner());
    }
}

