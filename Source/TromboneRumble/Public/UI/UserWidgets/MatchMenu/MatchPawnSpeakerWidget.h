// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "MatchPawnSpeakerWidget.generated.h"

class UImage;
class UTromboneVOIPTalker;

/**
 * Speaker indicator embedded inside the player nameplate (WBP_PlayerNameplate).
 * Tints its image black (0,0,0,1) while silent and white (1,1,1,1) while the player is speaking.
 */
UCLASS()
class TROMBONERUMBLE_API UMatchPawnSpeakerWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	/** Bind to the given talker's talking-state delegate and apply the initial color. */
	void Init(UTromboneVOIPTalker* InTalker);

protected:
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Speaker;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	FLinearColor SpeakingColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	FLinearColor SilentColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.28f);

private:
	UFUNCTION()
	void HandleTalkingStateChanged(bool bIsTalking);

	void ApplyColor(bool bIsTalking);

	TWeakObjectPtr<UTromboneVOIPTalker> WeakTalker;
};
