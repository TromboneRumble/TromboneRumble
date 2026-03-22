#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/QuestData.h"
#include "Items/InstrumentBase.h"
#include "TutorialManager.generated.h"

class ATutorialDummy;
enum class ETutorialExtraDataType : uint8;
class UTromboneGameInstance;
class ADefaultTromboneCharacter;
enum class ENoteResult : uint8;
class URhythmSubsystem;
enum class EWeaponType : uint8;
enum class EQuestConditionParamType : uint8;
enum class EQuestConditionType : uint8;
struct FActiveQuestData;

struct FQuestUIData
{
	FString QuestID;
	FText Description;
	UTexture2D* Icon;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnTutorialDialogueSequence, const FText& /*DialogueString*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTutorialQuestSequence, const TArray<FQuestUIData>& /*QuestUIData*/);
DECLARE_MULTICAST_DELEGATE(FOnTutorialTransitionSequence);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, const FString& /*QuestID*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnShowExtraData, UTexture2D* /*Image*/);

UCLASS()
class TROMBONERUMBLE_API ATutorialManager : public AActor
{
	GENERATED_BODY()
	
public:
	ATutorialManager();
	
	FOnTutorialDialogueSequence OnDialogueSequence;
	FOnTutorialQuestSequence OnQuestSequence;
	FOnTutorialTransitionSequence OnTransitionSequence;
	FOnQuestCompleted OnQuestCompleted;
	FOnShowExtraData OnShowExtraData;
	
	virtual void BeginPlay() override;
	
	void ReportAction(EQuestConditionType Condition, EQuestConditionParamType ConditionParam_0, FString ConditionParam_1 = FString());
	bool IsClearAllActiveQuests() const;
	
	void InitializeTutorial();
	void ProcessTutorial();
	void ProcessDialogueSequence();
	void ProcessQuestSequence();
	void ProcessTransitionSequence();
	void ProcessSequenceSideEffect();
	
	void ShowExtraData(ETutorialExtraDataType ExtraDataType, const FString& ExtraDataPath);
	
	void SpawnInstruments();
	AInstrumentBase* SpawnInstrument(EWeaponType WeaponType);
	void DestroySpawnedInstruments();
	void SpawnDummyCharacter();
	void DestroySpawnedDummyCharacter();
	
	void ShowTutorialCompletePopup();
	
	ADefaultTromboneCharacter* GetPlayerCharacter() const;
	
	UFUNCTION()
	void HandleOnNoteDetected(ENoteResult NoteResult);
	UFUNCTION()
	void HandleOnSpotlightBonusEarned();
	
protected:
	
	/** Currently active quests. Key is QuestID (1001 ...) */
	UPROPERTY()
	TMap<FString, FActiveQuestData> CurrentActiveQuest;
	
	/** Tutorial sequence Row names. Key is TutorialSequence_001 ... */
	UPROPERTY()
	TArray<FName> TutorialSequenceNames;
	
	/** Tutorial sequence index */
	UPROPERTY()
	int32 CurrentIndex = -1;
	
	/** Timer handle for ProcessTutorial() delay */
	FTimerHandle TimerHandle_Tutorial;
	
	/** Quest sequence is currently in progress. Prevents processing sequence while quest */
	bool bIsQuestSequenceProcessing = false;
	
	/** Interval after quest completion before processing the next tutorial sequence (seconds) */
	float IntervalAfterQuestCompletion = 1.5f;
	
	UPROPERTY()
	TObjectPtr<URhythmSubsystem> RhythmSubsystem;
	UPROPERTY()
	TObjectPtr<UTromboneGameInstance> TromboneGameInstance;
	
	UPROPERTY(EditAnywhere, Category = "Tutorial")
	TObjectPtr<UDataTable> TutorialDataTable;
	
	UPROPERTY(EditAnywhere, Category = "Tutorial")
	TObjectPtr<UDataTable> QuestDataTable;
	
	/** Spawned instrument actors during the tutorial, used for destroy */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> SpawnedInstruments;
	
	/** Spawned dummy character during the tutorial, used for destroy */
	UPROPERTY()
	TObjectPtr<ATutorialDummy> SpawnedDummyCharacter;
	
	// TODO : CheatManager랑 함께 공유? 관리?
	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
	TMap<EWeaponType, TSubclassOf<AActor>> WeaponClasses;
	
	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
	TMap<EWeaponType, FVector> WeaponSpawnLocations;
	
	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
	TSubclassOf<AActor> DummyCharacterClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
	FVector DummyCharacterSpawnLocation;
};