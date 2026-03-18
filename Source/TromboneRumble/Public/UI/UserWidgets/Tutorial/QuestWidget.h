// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "QuestWidget.generated.h"

struct FQuestUIData;
class UImage;
class ATutorialManager;
class UCommonTextBlock;

enum class EQuestStatus : uint8
{
	InProgress,
	Completed,
};

UCLASS()
class TROMBONERUMBLE_API UQuestWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	
	/** Default constructor. */
	UQuestWidget();
	
	/** Initializes the quest widget with the given description and icon. */
	void InitQuestWidget(const FQuestUIData& QuestUIData);
	void InitQuestWidget(const FText& Description, UTexture2D* Icon);
	
	/** Updates the quest status and changes the status icon. */
	void UpdateQuestStatus(EQuestStatus NewStatus);
	
protected:
	
	void HandleInProgressStatus();
	
	void HandleCompletedStatus();
	
protected:
	
	/** Quest description */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> CT_Description;
	
	/** Quest icon */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Icon;
	
	/** complete or in-progress status icon */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Status;
	
	/** Current quest status */
	EQuestStatus CurrentStatus;
	
	/** Unique Quest ID */
	FString QuestID;
	
public:
	
	/** @return Unique Quest ID */
	FString GetQuestID() const { return QuestID; }
	
};
