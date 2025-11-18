// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "DefaultPlayerState.generated.h"

class AInstrumentBase;

UCLASS()
class TROMBONERUMBLE_API ADefaultPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ADefaultPlayerState();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_PlayerName() override;
	virtual void CopyProperties(APlayerState* PlayerState) override;

	void SetIsReady(bool bReady);
	FORCEINLINE bool IsReady() const { return bIsReady; }

	UPROPERTY(VisibleInstanceOnly, Replicated)
	TSubclassOf<AInstrumentBase> EquippedInstrumentClass;

private:
	UPROPERTY(Replicated)
	bool bIsReady = false;
};
