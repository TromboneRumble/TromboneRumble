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

    UInteractionTriggerComponent* Trigger = Target->FindComponentByClass<UInteractionTriggerComponent>();
    if (!Trigger) return;

    if (GetOwnerRole() != ROLE_Authority && !Trigger->IsTriggerActive()) return;

    Server_TryInteract(Trigger);
}

void UInteractorComponent::RegisterCandidate(AActor* InCandidate)
{
    if (!IsValid(InCandidate)) return;

    for (auto& Candidate : Candidates)
    {
        if (Candidate.Get() == InCandidate) return;
    }
    Candidates.Add(InCandidate);

    AActor* NewBest = GetBestCandidate();
    if (NewBest)
    {
        OnInteractableAvailable.Broadcast(true);
    }
    else
    {
        OnInteractableAvailable.Broadcast(false);
    }
    
    if (NewBest != BestCandidateCached.Get())
    {
        BestCandidateCached = NewBest;
        OnBestCandidateChanged.Broadcast(NewBest);
    }
}

void UInteractorComponent::UnregisterCandidate(AActor* InCandidate)
{
    bool bRemoved = false;
    for (int32 i = Candidates.Num() - 1; i >= 0; --i)
    {
        if (Candidates[i].Get() == InCandidate)
        {
            Candidates.RemoveAt(i);
            bRemoved = true;
        }
        else if (!Candidates[i].IsValid())
        {
            Candidates.RemoveAt(i);
        }
    }

    const bool bAny = Candidates.Num() > 0;
    OnInteractableAvailable.Broadcast(bAny);

    AActor* NewBest = GetBestCandidate();
    if (NewBest != BestCandidateCached.Get())
    {
        BestCandidateCached = NewBest;
        OnBestCandidateChanged.Broadcast(NewBest);
    }
}

void UInteractorComponent::Client_OnInteractSuccess_Implementation(AActor* InteractedActor)
{
    if (InteractedActor)
    {
        OnInteractSuccessDelegate.Broadcast(InteractedActor);
    }
}

void UInteractorComponent::Server_TryInteract_Implementation(UInteractionTriggerComponent* TriggerToInteract)
{
    if (IsValid(TriggerToInteract))
    {
        TriggerToInteract->Server_TryInteract(GetOwner());
    }
}

AActor* UInteractorComponent::GetBestCandidate() const
{
    // 마지막으로 들어온 후보를 최우선
    for (int32 i = Candidates.Num() - 1; i >= 0; --i)
    {
        if (AActor* A = Candidates[i].Get())
        {
            if (A->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
            {
                return A;
            }
        }
    }
    return nullptr;
}

void UInteractorComponent::CleanupCandidates()
{
    for (int32 i = Candidates.Num() - 1; i >= 0; --i)
    {
        if (!Candidates[i].IsValid())
        {
            Candidates.RemoveAt(i);
        }
    }
}