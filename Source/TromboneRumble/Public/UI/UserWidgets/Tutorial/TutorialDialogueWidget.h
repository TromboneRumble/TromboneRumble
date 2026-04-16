#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "TutorialDialogueWidget.generated.h"

class UImage;
class UCommonTextBlock;

UCLASS()
class TROMBONERUMBLE_API UTutorialDialogueWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

protected:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> CT_Dialogue;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Speaker;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> BounceAnim;
	
private:
	
	UFUNCTION()
	void OnTutorialDialogueSequence(const FText& DialogueString);

protected:

	// ~ Begin UUserWidget interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	// ~ End UUserWidget interface
	
};
