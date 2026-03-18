// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "TutorialQuestWidget.generated.h"

class ATutorialManager;
class UQuestWidget;
struct FQuestUIData;
class UDynamicEntryBox;

UCLASS()
class TROMBONERUMBLE_API UTutorialQuestWidget : public UCommonActivatableWidget
{
	
	GENERATED_BODY()
public:
	
	/** Default constructor. */
	UTutorialQuestWidget();
	
	/** Creates a quest widget for the given quest data. */
	virtual void CreateQuestWidget(const FQuestUIData& QuestUIData);
	
	/** Removes the quest widget of an existing quest. */
	virtual void ClearQuestWidgets();
	
public:
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UQuestWidget> QuestWidgetClass;
	
protected:
	
	/** Dynamic entry box that handles quest widgets. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox> DEB_QuestList;
	
private:
	
	/** Handles the quest sequence event from the tutorial manager and creates quest widgets. */
	void HandleQuestSequence(const TArray<FQuestUIData>& QuestUIDataArray);
	
	void HandleQuestCompleted(const FString& QuestID);

private:
	
	/* Reference to the tutorial manager */
	UPROPERTY()
	TObjectPtr<ATutorialManager> TutorialManager;
	
public:
	
	//~ Begin UUserWidget Interface
	virtual void NativeOnInitialized() override;
	//~ End UUserWidget Interface
	
};
