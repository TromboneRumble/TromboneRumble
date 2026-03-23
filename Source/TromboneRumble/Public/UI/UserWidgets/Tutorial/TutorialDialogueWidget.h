// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "TutorialDialogueWidget.generated.h"

class UImage;
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

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Speaker;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> BounceAnim;
	
private:
	void SetDialogueText(const FText& DialogueString);

	/** Reference to the tutorial manager */
	UPROPERTY()
	TObjectPtr<ATutorialManager> TutorialManager;
};
