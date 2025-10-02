// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractorComponent.generated.h"

/// <summary>
/// IInteractable이 있는 액터와 상호작용할수 있는 컴포넌트
/// IInteractable이 구현되어있는 액터는 InteractionTriggerComponent가 있어야함
/// </summary>
UCLASS( ClassGroup=(Interaction), meta=(BlueprintSpawnableComponent, DisableNativeTick) )
class TROMBONERUMBLE_API UInteractorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractorComponent();

    /// <summary>
    /// 명시적으로 지정된 대상 또는 기본 대상과 상호작용을 시도
    /// </summary>
    /// <param name="ExplicitTarget">상호작용을 시도할 명시적인 대상 액터. nullptr이면 기본 대상과 상호작용</param>
    UFUNCTION(BlueprintCallable, Category = "Interact")
    void TryInteract(AActor* ExplicitTarget = nullptr);

    // UInteractionTriggerComponent
    void RegisterCandidate(AActor* Candidate);
    void UnregisterCandidate(AActor* Candidate);
    // ~UInteractionTriggerComponent

    FORCEINLINE const TArray<TWeakObjectPtr<AActor>>& GetCandidates() { return Candidates; }

    // TODO : 후보 선정 정책(거리/정면각/라인오브사이트 등) 나중에 확장
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
    bool bPreferLastEntered = true;
private:
    AActor* GetBestCandidate() const;

    UFUNCTION(Server, Reliable)
    void Server_TryInteract(AActor* Target);

    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> Candidates;

    
		
};
