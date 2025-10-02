// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interact를 실행하는 주체는 Actor에 UInteractorComponent를 가지고 있어야함
 * Interact 대상은 IInteractable 인터페이스를 구현해야하고, InteractionTriggerComponent를 가지고 있어야함
 */
class TROMBONERUMBLE_API IInteractable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact")
	bool CanInteract(AActor* InstigatorActor) const;

	// 실제 상호작용 실행(E 키 등)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact")
	void Interact(AActor* InstigatorActor);
};
