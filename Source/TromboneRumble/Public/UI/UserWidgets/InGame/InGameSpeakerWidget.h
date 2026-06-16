// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "InGameSpeakerWidget.generated.h"

class UTromboneVOIPTalker;

/**
 * Speaker indicator shown above a character's head in-game (WBP_SpeakerIndicator).
 * Shows itself while the player is speaking (PTT or auto) and hides otherwise.
 */
UCLASS()
class TROMBONERUMBLE_API UInGameSpeakerWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	/** Bind to the given talker's talking-state delegate and apply the initial visibility. */
	void Init(UTromboneVOIPTalker* InTalker);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void HandleTalkingStateChanged(bool bIsTalking);

	/** (Re)bind to the cached talker. Called from Init and on every NativeConstruct so the
	 *  binding survives the screen-space widget component's repeated add/remove churn. */
	void BindToTalker();

	void ApplyVisible(bool bIsTalking);

	TWeakObjectPtr<UTromboneVOIPTalker> WeakTalker;
};
