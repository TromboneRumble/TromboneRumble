#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/QuestData.h"
#include "TutorialManager.generated.h"

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

DECLARE_MULTICAST_DELEGATE_OneParam(FOnTutorialDialogueSequence, const FString& /*DialogueString*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTutorialQuestSequence, const TArray<FQuestUIData>& /*QuestUIData*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, const FString& /*QuestID*/);

UCLASS()
class TROMBONERUMBLE_API ATutorialManager : public AActor
{
	GENERATED_BODY()
	
public:
	ATutorialManager();
	
	FOnTutorialDialogueSequence OnDialogueSequence;
	FOnTutorialQuestSequence OnQuestSequence;
	FOnQuestCompleted OnQuestCompleted;
	
	virtual void BeginPlay() override;
	
	void ReportAction(EQuestConditionType Condition, EQuestConditionParamType ConditionParam_0, FString ConditionParam_1 = FString());
	bool IsClearAllActiveQuests() const;
	
	void InitializeTutorial();
	void ProcessTutorial();
	void ProcessDialogueSequence();
	void ProcessQuestSequence();
	void InternalProcessQuestSequence();
	void ProcessTransitionSequence();
	
	void SpawnInstruments();
	void SpawnDummyCharacter();
	
protected:
	
	UPROPERTY(VisibleAnywhere)
	TMap<FString, FActiveQuestData> CurrentActiveQuest;
	
	UPROPERTY()
	TArray<FName> TutorialSequenceNames;
	
	int32 CurrentIndex = -1;
	
	FTimerHandle TimerHandle_Tutorial;
	
	UPROPERTY()
	TObjectPtr<URhythmSubsystem> RhythmSubsystem;
	
	UPROPERTY(EditAnywhere, Category = "Tutorial")
	TObjectPtr<UDataTable> TutorialDataTable;
	
	UPROPERTY(EditAnywhere, Category = "Tutorial")
	TObjectPtr<UDataTable> QuestDataTable;
	
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