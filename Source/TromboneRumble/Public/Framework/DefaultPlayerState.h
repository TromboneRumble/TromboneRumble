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

	UPROPERTY(VisibleInstanceOnly, Replicated)
	TSubclassOf<AInstrumentBase> EquippedInstrumentClass;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_SkinColor)
	FLinearColor SkinColor = FLinearColor::Black;

	UFUNCTION()
	void OnRep_SkinColor();

private:
	UPROPERTY(Replicated)
	bool bIsReady = false;

public:
	// Getter & Setter
	void SetIsReady(bool bReady);
	void SetSkinColor(const FLinearColor& InSkinColor);
	
	FORCEINLINE bool IsReady() const { return bIsReady; }
	FORCEINLINE FLinearColor GetSkinColor() const { return SkinColor; }
	// ~ Getter & Setter
};
