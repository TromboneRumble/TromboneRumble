#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Data/QuestData.h"
#include "TutorialWorldSubsystem.generated.h"

class ATutorialManager;
enum class ENoteResult : uint8;
class URhythmSubsystem;
enum class ETutorialExtraDataType : uint8;
class UTromboneGameInstance;
enum class EQuestConditionParamType : uint8;
enum class EQuestConditionType : uint8;

USTRUCT()
struct FQuestUIData
{
	GENERATED_BODY()
	
	FQuestUIData() : Icon(nullptr) {}
	
	FQuestUIData(const FString& InQuestID, const FText& InDescription, UTexture2D* InIcon)
		: QuestID(InQuestID), Description(InDescription), Icon(InIcon) {}
	
	UPROPERTY()
	FString QuestID;
	
	UPROPERTY()
	FText Description;
	
	UPROPERTY()
	TObjectPtr<UTexture2D> Icon;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FK2_OnTutorialDialogueSequence, const FText&, DialogueString);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FK2_OnTutorialQuestSequence, const TArray<FQuestUIData>&, QuestUIDataArray);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FK2_OnTutorialTransitionSequence);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FK2_OnQuestCompleted, const FString&, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FK2_OnShowExtraData, UTexture2D*, TutorialImage);

UCLASS()
class TROMBONERUMBLE_API UTutorialWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	
	/** Default constructor. */
	UTutorialWorldSubsystem();
	
	/** Proceed to the next tutorial step */
	void ProcessTutorial();
	
	/** Reports action related to quests */
	void ReportAction(EQuestConditionType Condition, EQuestConditionParamType ConditionParam_0, FString ConditionParam_1 = FString());
	
	/** @return true if there are no active quests or all active quests are completed */
	bool IsClearAllActiveQuests() const;
	
private:
	
	void ProcessDialogueSequence();
	void ProcessQuestSequence();
	void ProcessTransitionSequence();
	
	/** Handle additional actions, such as spawning instrument, and activating gimmicks */
	void ProcessSequenceEvent();
	
	/** Show tutorial image or video */
	void ShowExtraData(ETutorialExtraDataType ExtraDataType, const FString& ExtraDataPath) const;

private:
	
	UFUNCTION()
	void HandleOnNoteDetected(ENoteResult NoteResult);
	
private:
	
	/** Timer handle for ProcessTutorial() delay */
	FTimerHandle TimerHandle_Tutorial;
	
	/** Quest sequence is currently in progress. Prevents processing sequence while quest */
	bool bIsQuestSequenceProcessing;
	
	/** Interval after quest completion before processing the next tutorial sequence */
	float IntervalAfterQuestCompletion;
	
private:
	
	/** Currently active quests. Key is QuestID (1001 ...) */
	UPROPERTY()
	TMap<FString, FActiveQuestData> CurrentActiveQuest;
	
	/** Tutorial sequence Row names. Key is TutorialSequence_001 ... */
	UPROPERTY()
	TArray<FName> TutorialSequenceNames;
	
	/** Tutorial sequence index */
	UPROPERTY()
	int32 CurrentIndex = -1;
	
	TWeakObjectPtr<ATutorialManager> TutorialManager;
	
	UPROPERTY()
	TObjectPtr<UDataTable> TutorialDataTable;
	
	UPROPERTY()
	TObjectPtr<UDataTable> QuestDataTable;

public:
	
	/** Event when each dialogue is displayed */
	UPROPERTY(BlueprintAssignable, Category = "Events", DisplayName = "On Tutorial Dialogue Sequence")
	FK2_OnTutorialDialogueSequence OnDialogueSequenceEvent;
	
	/** Event when each quest is given */
	UPROPERTY(BlueprintAssignable, Category = "Events", DisplayName = "On Tutorial Quest Sequence")
	FK2_OnTutorialQuestSequence OnQuestSequenceEvent;
	
	/** Event when transition sequence is triggered */
	UPROPERTY(BlueprintAssignable, Category = "Events", DisplayName = "On Tutorial Transition Sequence")
	FK2_OnTutorialTransitionSequence OnTransitionSequenceEvent;
	
	/** Event when a quest is completed */
	UPROPERTY(BlueprintAssignable, Category = "Events", DisplayName = "On Quest Completed")
	FK2_OnQuestCompleted OnQuestCompletedEvent;
	
	/** Event when extra data for additional tutorial description is displayed */
	UPROPERTY(BlueprintAssignable, Category = "Events", DisplayName = "On Show Tutorial Extra Data")
	FK2_OnShowExtraData OnShowExtraDataEvent;
	
private:
	
	/** @return RhythmSubsystem */
	URhythmSubsystem* GetCachedRhythmSubsystem();
	
	TWeakObjectPtr<URhythmSubsystem> CachedRhythmSubsystem = nullptr;
	
	UPROPERTY()
	TObjectPtr<UTromboneGameInstance> CachedTromboneGameInstance = nullptr;
	
public:
	
	void RegisterManager(ATutorialManager* InManager);
	void UnregisterManager();
	
public:
	
	// ~ Begin UWorldSubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	// ~ End UWorldSubsystem Interface
	
};
