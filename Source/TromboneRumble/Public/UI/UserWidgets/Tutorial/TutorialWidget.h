#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "TutorialWidget.generated.h"

class UCommonBorder;
class UImage;
class UBaseUIRoot;
struct FQuestUIData;
class UTutorialQuestWidget;
class UTutorialDialogueWidget;
class ATutorialManager;

UCLASS()
class TROMBONERUMBLE_API UTutorialWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	/** Default constructor. */
	UTutorialWidget();
	
protected:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTutorialDialogueWidget> WBP_Dialogue;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTutorialQuestWidget> WBP_Quest;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_ExtraData;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonBorder> Border_Dim;
	
	/** Input actions to skip dialogue */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TArray<FDataTableRowHandle> SkipActionRowArray;
	
private:
	
	/** Reference to the tutorial manager */
	UPROPERTY()
	TObjectPtr<ATutorialManager> TutorialManager;
	
	/** Reference to the root UI layout */
	UPROPERTY()
	TObjectPtr<UBaseUIRoot> RootLayout;
	
	/** UI action binding handles for skipping dialogue */
	UPROPERTY()
	TArray<FUIActionBindingHandle> SkipActionHandles;
	
private:
	
	/** Registers input actions for skipping dialogue. */
	void RegisterInputActions();
	
	/** Unregisters input actions for skipping dialogue. */
	void UnregisterInputActions();
	
	/** Handles the dialogue sequence event */
	void HandleDialogueSequence(const FText& DialogueString);
	
	/** Handles the quest sequence event */
	void HandleQuestSequence(const TArray<FQuestUIData>& QuestUIDataArray);
	
	/** Handles the transition sequence event */
	void HandleTransitionSequence();
	
	/** Handles the skip dialogue input action */
	void HandleSkipDialogue();
	
	/** Handles the show extra data event */
	void HandleOnExtraData(UTexture2D* Image);
	
private:
	
	/** Set visibility of the tutorial UI */
	void SetUIVisibility(ESlateVisibility NewVisibility);
	
protected:
	// ~ Begin UCommonActivatableWidget Interface
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	// ~ End UCommonActivatableWidget Interface
};
