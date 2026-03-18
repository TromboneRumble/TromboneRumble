// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "TutorialWidget.generated.h"

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
	
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
protected:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTutorialDialogueWidget> WBP_Dialogue;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTutorialQuestWidget> WBP_Quest;
	
	/** Input actions to skip dialogue */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TArray<FDataTableRowHandle> SkipActionRowArray;
	
private:
	
	/** Reference to the tutorial manager */
	UPROPERTY()
	TObjectPtr<ATutorialManager> TutorialManager;
	
	/** UI action binding handles for skipping dialogue */
	UPROPERTY()
	TArray<FUIActionBindingHandle> SkipActionHandles;
	
private:
	
	/** Registers input actions for skipping dialogue. */
	void RegisterInputActions();
	
	/** Unregisters input actions for skipping dialogue. */
	void UnregisterInputActions();
	
	/** Handles the dialogue sequence event */
	void HandleDialogueSequence(const FString& DialogueString);
	
	/** Handles the quest sequence event */
	void HandleQuestSequence(const TArray<FQuestUIData>& QuestUIDataArray);
	
	/** Handles the skip dialogue input action */
	void HandleSkipDialogue();

};
