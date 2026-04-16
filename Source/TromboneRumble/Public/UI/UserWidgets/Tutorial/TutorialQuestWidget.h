#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "TutorialQuestWidget.generated.h"

class UQuestWidget;
struct FQuestUIData;
class UDynamicEntryBox;

UCLASS()
class TROMBONERUMBLE_API UTutorialQuestWidget : public UCommonActivatableWidget
{
	
	GENERATED_BODY()
public:
	
	/** Creates a quest widget for the given quest data. */
	void CreateQuestWidget(const FQuestUIData& QuestUIData);
	
	/** Removes the quest widget of an existing quest. */
	void ClearQuestWidgets();
	
public:
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UQuestWidget> QuestWidgetClass;
	
protected:
	
	/** Dynamic entry box that handles quest widgets. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox> DEB_QuestList;
	
private:
	
	/** Handles the quest sequence event from the tutorial manager and creates quest widgets. */
	UFUNCTION()
	void HandleQuestSequence(const TArray<FQuestUIData>& QuestUIDataArray);
	
	/** Handles the quest completed event from the tutorial manager and updates the corresponding quest widget. */
	UFUNCTION()
	void HandleQuestCompleted(const FString& QuestID);

public:
	
	//~ Begin UUserWidget Interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	//~ End UUserWidget Interface
	
};
