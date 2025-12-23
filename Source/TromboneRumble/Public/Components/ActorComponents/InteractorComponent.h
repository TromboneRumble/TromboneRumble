// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractorComponent.generated.h"

class UInteractionTriggerComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractSuccessSignature, AActor*, InteractedActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractableAvailable, bool, bAvailable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBestCandidateChanged, AActor*, NewTarget);

/// <summary>
/// IInteractable이 있는 액터와 상호작용할수 있는 컴포넌트
/// IInteractable이 구현되어있는 액터는 InteractionTriggerComponent가 있어야함
/// </summary>
UCLASS()
class TROMBONERUMBLE_API UInteractorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractorComponent();

    void TryInteract(const AActor* ExplicitTarget = nullptr);
    void RegisterCandidate(AActor* InCandidate);
    void UnregisterCandidate(AActor* InCandidate);

    FOnInteractableAvailable OnInteractableAvailable;
    FOnBestCandidateChanged OnBestCandidateChanged;
	FOnInteractSuccessSignature OnInteractSuccessDelegate;
	
	UFUNCTION(Client, Reliable)
	void Client_OnInteractSuccess(AActor* InteractedActor);
	
protected:
    UFUNCTION(Server, Reliable)
	void Server_TryInteract(UInteractionTriggerComponent* TriggerToInteract);
	
private:
    AActor* GetBestCandidate() const;
    void CleanupCandidates();

    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> Candidates;

    UPROPERTY()
    TWeakObjectPtr<AActor> BestCandidateCached;
};