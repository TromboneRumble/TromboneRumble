// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "TutorialDialogueWidget.generated.h"

class ATutorialManager;
class UCommonTextBlock;

UCLASS()
class TROMBONERUMBLE_API UTutorialDialogueWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	/** Default constructor. */
	UTutorialDialogueWidget();
	
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> CT_Dialogue;
	
private:
	void SetDialogueText(const FString& DialogueString);

	/** Reference to the tutorial manager */
	UPROPERTY()
	TObjectPtr<ATutorialManager> TutorialManager;
};
