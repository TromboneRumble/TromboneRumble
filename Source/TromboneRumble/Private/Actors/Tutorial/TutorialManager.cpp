#include "Actors/Tutorial/TutorialManager.h"
#include "Data/QuestData.h"
#include "Data/TutorialData.h"
#include "Subsystems/RhythmSubsystem.h"
#include "Utilities/DebugHelper.h"
#include "Utilities/EnumHelper.h"

ATutorialManager::ATutorialManager()
{
}

void ATutorialManager::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeTutorial();

	const FTimerDelegate TimerDelegate = FTimerDelegate::CreateLambda([this]()
	{
		ProcessTutorial();
	});
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_Tutorial, TimerDelegate, 1.0f, false);
	
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		if (URhythmSubsystem* Subsystem = GameInstance->GetSubsystem<URhythmSubsystem>())
		{
			RhythmSubsystem = Subsystem;
		}
	}
	
	if (!RhythmSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("RhythmSubsystem is null"));
	}
}

void ATutorialManager::ReportAction(EQuestConditionType Condition, EQuestConditionParamType ConditionParam_0,
	FString ConditionParam_1)
{
	const FString DebugMsg = FString::Printf(TEXT("Condition: %s, ConditionParam_0: %s, ConditionParam_1: %s"), 
		*EnumHelper::EnumToString(Condition), 
		*EnumHelper::EnumToString(ConditionParam_0), *ConditionParam_1);
	// PRINT_WITH_CURRENT_CONTEXT(DebugMsg);
	bool bQuestUpdated = false;
	
	for (auto& Elem : CurrentActiveQuest)
	{
		FActiveQuestData& ActiveQuestData = Elem.Value;
		
		if (ActiveQuestData.bIsCompleted) continue;
		
		PRINT_WITH_CURRENT_CONTEXT(FString::Printf(TEXT("Param_1 - Active: %s, Reported: %s"), *ActiveQuestData.QuestConditionParam_1, *ConditionParam_1));
		
		if (ActiveQuestData.QuestCondition == Condition &&
			ActiveQuestData.QuestConditionParam_0 == ConditionParam_0 &&
			ActiveQuestData.QuestConditionParam_1 == ConditionParam_1)
		{
			ActiveQuestData.QuestCondition_Count--;
			bQuestUpdated = true;
			
			if (ActiveQuestData.QuestCondition_Count <= 0)
			{
				ActiveQuestData.bIsCompleted = true;
				OnQuestCompleted.Broadcast(ActiveQuestData.QuestID);
				UE_LOG(LogTemp, Log, TEXT("Quest %s completed!"), *ActiveQuestData.QuestID);
			}
		}
	}
	
	if (bQuestUpdated)
	{
		if (IsClearAllActiveQuests())
		{
			UE_LOG(LogTemp, Log, TEXT("All quests completed! Proceeding to the next tutorial sequence."));
			ProcessTutorial();
		}
	}
}

bool ATutorialManager::IsClearAllActiveQuests() const
{
	for (const auto& Elem : CurrentActiveQuest)
	{
		const FActiveQuestData& ActiveQuestData = Elem.Value;
		if (!ActiveQuestData.bIsCompleted)
		{
			return false;
		}
	}
	return true;
}

void ATutorialManager::InitializeTutorial()
{
	if (!TutorialDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("TutorialDataTable is not set"));
		return;
	}
	
	TutorialSequenceNames = TutorialDataTable->GetRowNames();

	TutorialSequenceNames.Sort([this](const FName& A, const FName& B) {
		const FString Context;
		const FTutorialData* DataA = TutorialDataTable->FindRow<FTutorialData>(A, Context);
		const FTutorialData* DataB = TutorialDataTable->FindRow<FTutorialData>(B, Context);

		if (DataA && DataB)
		{
			return DataA->TID < DataB->TID;
		}
		return false;
	});

	CurrentIndex = 0;
}

void ATutorialManager::ProcessTutorial()
{
	if (CurrentIndex >= TutorialSequenceNames.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("Tutorial sequence completed"));
		return;
	}
	
	const FName CurrentSequenceName = TutorialSequenceNames[CurrentIndex];
	const ETutorialSequenceType SequenceType = TutorialDataTable->FindRow<FTutorialData>(CurrentSequenceName, FString())->SequenceType;

	switch (SequenceType)
	{
		case ETutorialSequenceType::Dialogue:
			ProcessDialogueSequence();
			break;
		case ETutorialSequenceType::Quest:
			ProcessQuestSequence();
			break;
		case ETutorialSequenceType::Transition:
			ProcessTransitionSequence();
			break;
		default:
			UE_LOG(LogTemp, Warning, TEXT("Unknown tutorial sequence type"));
			break;
	}
	
	CurrentIndex++;
}

void ATutorialManager::ProcessDialogueSequence()
{
	FString Dialogue = TutorialDataTable->FindRow<FTutorialData>(TutorialSequenceNames[CurrentIndex], FString())->DummyDialogue;

	OnDialogueSequence.Broadcast(Dialogue);
}

void ATutorialManager::ProcessQuestSequence()
{
	TArray<FString> QuestIDs = TutorialDataTable->FindRow<FTutorialData>(TutorialSequenceNames[CurrentIndex], FString())->QuestID;
	TArray<FQuestUIData> QuestUIDataArray;
	CurrentActiveQuest.Empty();
	
	for (const FString& QuestID : QuestIDs)
	{
		FQuestData* QuestData = QuestDataTable->FindRow<FQuestData>(FName(*QuestID), FString());
		if (QuestData)
		{
			FActiveQuestData ActiveQuestData;
			ActiveQuestData.QuestID = QuestID;
			ActiveQuestData.QuestCondition = QuestData->QuestCondition;
			ActiveQuestData.QuestConditionParam_0 = QuestData->QuestConditionParam_0;
			ActiveQuestData.QuestConditionParam_1 = QuestData->QuestConditionParam_1;
			ActiveQuestData.QuestCondition_Count = QuestData->QuestCondition_Count;
			CurrentActiveQuest.Add(QuestID, ActiveQuestData);
			
			FText Description = FText::FromString(QuestData->Comment); // TODO:
			UTexture2D* Icon = LoadObject<UTexture2D>(nullptr, *QuestData->IconPath);
			
			if (!Icon)
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to load icon for Quest ID %s from path %s"), *QuestID, *QuestData->IconPath);
			}
			
			QuestUIDataArray.Add({ QuestID, Description, Icon });
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Quest ID %s not found in QuestDataTable"), *QuestID);
		}
	}
	
	OnQuestSequence.Broadcast(QuestUIDataArray);
	
	if (TutorialSequenceNames[CurrentIndex] == FName("TutorialSequence_016"))
	{
		SpawnInstruments();
		RhythmSubsystem->StartRhythmGame();
	}
	else if (TutorialSequenceNames[CurrentIndex] == FName("TutorialSequence_017"))
	{
		RhythmSubsystem->PauseRhythmGame();
	}
	else if (TutorialSequenceNames[CurrentIndex] == FName("TutorialSequence_021"))
	{
		SpawnDummyCharacter();
	}
	else if (TutorialSequenceNames[CurrentIndex] == FName("TutorialSequence_030"))
	{
		RhythmSubsystem->ResumeRhythmGame();
	}
}

void ATutorialManager::InternalProcessQuestSequence()
{
}

void ATutorialManager::ProcessTransitionSequence()
{
	FTimerDelegate TimerDelegate = FTimerDelegate::CreateLambda([this]()
	{
		ProcessTutorial();
	});
	
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_Tutorial, TimerDelegate, 1.0f, false);
}

void ATutorialManager::SpawnInstruments()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	for (const auto Elem : WeaponClasses)
	{
		const EWeaponType WeaponType = Elem.Key;
		TSubclassOf<AActor> WeaponClass = Elem.Value;
		if (!WeaponClass) continue;

		FVector SpawnLocation = WeaponSpawnLocations.Contains(WeaponType) ? WeaponSpawnLocations[WeaponType] : FVector::ZeroVector;
		FRotator SpawnRotation = FRotator::ZeroRotator;
		
		const AActor* SpawnedInstrument = GetWorld()->SpawnActor<AActor>(WeaponClass, SpawnLocation, SpawnRotation, SpawnParams);
		if (!SpawnedInstrument)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to spawn instrument of class %s"), *WeaponClass->GetName());
		}
	}
}

void ATutorialManager::SpawnDummyCharacter()
{
	if (!DummyCharacterClass) return;
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FRotator Rotation = FRotator(0, 180, 0);
	const AActor* SpawnedDummy = GetWorld()->SpawnActor<AActor>(DummyCharacterClass, DummyCharacterSpawnLocation, Rotation, SpawnParams);
	if (!SpawnedDummy)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to spawn dummy character of class %s"), *DummyCharacterClass->GetName());
	}
}	