// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionTriggerComponent.generated.h"

class USphereComponent;
class UInteractorComponent;

/// <summary>
/// IInteractable이 구현되어 있는 액터를 트리거할 수 있는가 확인해주는 컴포넌트
/// </summary>
UCLASS()
class TROMBONERUMBLE_API UInteractionTriggerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInteractionTriggerComponent();

	// InteractComponent를 통해 호출
	// 상호작용 성공 시(서버에서만 호출): 트리거를 소비(비활성화)하고 모든 후보에서 제거
	UFUNCTION(Server, Reliable)
	void Server_TryInteract(AActor* InstigatorActor);
	// ~InteractComponent

	UFUNCTION(Server, Reliable)
	void SetTriggerActive(bool bActivate);
	
	FORCEINLINE bool IsTriggerActive() const { return bTriggerActive; }
	
protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	TObjectPtr<USphereComponent> TriggerVolume = nullptr;

private:
	void ProcessOverlap(AActor* OtherActor);

	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnRep_TriggerActive();

	void SetupCollision(UShapeComponent* Shape);
	void SetCollisionEnabled(bool bEnable);
	void ForceRemoveThisFromAllInteractors();

	UPROPERTY(Transient, VisibleAnywhere, Category = "Interact")
	TSet<TWeakObjectPtr<UInteractorComponent>> OverlappingInteractors;

	UPROPERTY(ReplicatedUsing = OnRep_TriggerActive)
	bool bTriggerActive = true;
};