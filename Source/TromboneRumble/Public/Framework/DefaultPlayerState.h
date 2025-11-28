// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "DefaultPlayerState.generated.h"

class AInstrumentBase;
class URhythmSubsystem;
class AInGameState;
enum class ENoteResult : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLocalScoreChanged, APlayerState*, PlayerState);
UCLASS()
class TROMBONERUMBLE_API ADefaultPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ADefaultPlayerState();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_PlayerName() override;
	virtual void OnRep_Score() override;
	virtual void CopyProperties(APlayerState* PlayerState) override;	


	UPROPERTY(BlueprintAssignable)
	FOnLocalScoreChanged OnLocalScoreChanged;

	void AddScore(int32 Amount);
	UFUNCTION(Server, Reliable)
	void Server_AddScore(int32 Amount);
	UFUNCTION()
	void HandleNoteDetected(ENoteResult InNoteResult);

	UPROPERTY(VisibleInstanceOnly, Replicated)
	TSubclassOf<AInstrumentBase> EquippedInstrumentClass;
	
protected:
	UPROPERTY(ReplicatedUsing = OnRep_SkinColor)
	FLinearColor SkinColor = FLinearColor::Black;

	UFUNCTION()
	void OnRep_SkinColor();


public:
	//getter setter
	FORCEINLINE float GetRhythmScore() const { return GetScore(); }
	void SetSkinColor(const FLinearColor& InSkinColor);
	FORCEINLINE FLinearColor GetSkinColor() const { return SkinColor; }
	// ~getter setter
};
