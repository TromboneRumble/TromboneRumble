#include "Subsystems/WorldSubsystem/TutorialWorldSubsystem.h"
#include "Actors/Gimmick/GimmickManager.h"
#include "Actors/Tutorial/TutorialManager.h"
#include "Data/TutorialData.h"
#include "DeveloperSettings/TromboneConfig.h"
#include "Framework/TromboneGameInstance.h"
#include "Kismet/KismetInternationalizationLibrary.h"
#include "Subsystems/RhythmSubsystem.h"
#include "Utilities/DebugHelper.h"

UTutorialWorldSubsystem::UTutorialWorldSubsystem()
	: bIsQuestSequenceProcessing(false),
	  IntervalAfterQuestCompletion(0.0f)
{
}

void UTutorialWorldSubsystem::ProcessTutorial()
{
	if (bIsQuestSequenceProcessing) return;
	
	if (CurrentIndex < 0)
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("CurrentIndex is negative. Failed"));
		return;
	}
	
	if (CurrentIndex >= TutorialSequenceNames.Num())
	{
		TutorialManager->ShowTutorialCompletePopup();
		return;
	}
	
	const FName CurrentSequenceName = TutorialSequenceNames[CurrentIndex];
	const ETutorialSequenceType SequenceType = TutorialDataTable->FindRow<FTutorialData>(CurrentSequenceName, FString())->SequenceType;
	
	LOG_WITH_CURRENT_CONTEXT(Log, FString::Printf(TEXT("Processing Sequence Index: %d, Name: %s"), CurrentIndex, *CurrentSequenceName.ToString()));
	
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
		LOG_WITH_CURRENT_CONTEXT(Warning, FString::Printf(TEXT("Unknown sequence type for sequence %s"), *CurrentSequenceName.ToString()));
		break;
	}
}

void UTutorialWorldSubsystem::ReportAction(const EQuestConditionType Condition, const EQuestConditionParamType ConditionParam_0, FString ConditionParam_1)
{
	bool bQuestUpdated = false;
	
	for (auto& Elem : CurrentActiveQuest)
	{
		FActiveQuestData& ActiveQuestData = Elem.Value;
		
		if (ActiveQuestData.bIsCompleted) continue;
		
		if (ActiveQuestData.QuestCondition == Condition &&
			ActiveQuestData.QuestConditionParam_0 == ConditionParam_0 &&
			ActiveQuestData.QuestConditionParam_1 == ConditionParam_1)
		{
			ActiveQuestData.QuestCondition_Count--;
			
			if (ActiveQuestData.QuestCondition_Count <= 0)
			{
				bQuestUpdated = true;
				ActiveQuestData.bIsCompleted = true;
				OnQuestCompletedEvent.Broadcast(ActiveQuestData.QuestID);
			}
		}
	}
	
	if (bQuestUpdated)
	{
		if (IsClearAllActiveQuests())
		{
			bIsQuestSequenceProcessing = false;
			
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Tutorial);
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_Tutorial, this, &ThisClass::ProcessTutorial, IntervalAfterQuestCompletion, false);
		}
	}
}

bool UTutorialWorldSubsystem::IsClearAllActiveQuests() const
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

void UTutorialWorldSubsystem::ProcessDialogueSequence()
{
	const FTutorialData* TutorialData = TutorialDataTable->FindRow<FTutorialData>(TutorialSequenceNames[CurrentIndex], FString());
	if (!TutorialData) 
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("Tutorial Data is Null"));
		return;
	}
	
	const FString DescriptionStringId = TutorialData->DialogueStringID;
	FText Dialogue = FText::GetEmpty();
	if (CachedTromboneGameInstance)
	{
		Dialogue = CachedTromboneGameInstance->GetTutorialUIText(DescriptionStringId);
	}
	
	ProcessSequenceEvent();
	CurrentIndex++;
	OnDialogueSequenceEvent.Broadcast(Dialogue);
		
	const ETutorialExtraDataType ExtraDataType = TutorialData->ExtraDataType;
	FString ExtraDataPath;

	const FString CurrentCulture = UKismetInternationalizationLibrary::GetCurrentLanguage();
	if (CurrentCulture.Equals(TEXT("ko")))
	{
		ExtraDataPath = TutorialData->ExtraDataPath_ko;
	}
	else if (CurrentCulture.Equals(TEXT("en")))
	{
		ExtraDataPath = TutorialData->ExtraDataPath_en;
	}
	else
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, FString::Printf(TEXT("Unsupported culture %s. Defaulting to English extra data path."), *CurrentCulture));
		ExtraDataPath = TutorialData->ExtraDataPath_en;
	}
		
	ShowExtraData(ExtraDataType, ExtraDataPath);
}

void UTutorialWorldSubsystem::ProcessQuestSequence()
{
	bIsQuestSequenceProcessing = true;
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
			
			FString DescriptionStringId = QuestData->StringID;
			FText Description = FText::GetEmpty();
			if (CachedTromboneGameInstance)
			{
				Description = CachedTromboneGameInstance->GetTutorialUIText(DescriptionStringId);
			}
			
			FFormatNamedArguments FormatArgs = { 
				{ TEXT("1"), FText::FromString(QuestData->QuestConditionParam_1) },
				{ TEXT("0"), QuestData->QuestCondition_Count }
			};
			const FText FormattedDescription = FText::Format(Description, FormatArgs);
			UTexture2D* Icon = LoadObject<UTexture2D>(nullptr, *QuestData->IconPath);
			
			if (!Icon)
			{
				LOG_WITH_CURRENT_CONTEXT(Warning, FString::Printf(TEXT("Failed to load icon for Quest ID %s from path %s"), *QuestID, *QuestData->IconPath));
			}
			
			QuestUIDataArray.Add({ QuestID, FormattedDescription, Icon });
		}
		else
		{
			LOG_WITH_CURRENT_CONTEXT(Warning, FString::Printf(TEXT("Quest ID %s not found in QuestDataTable"), *QuestID));
		}
	}
	
	ProcessSequenceEvent();
	CurrentIndex++;
	OnQuestSequenceEvent.Broadcast(QuestUIDataArray);
}

void UTutorialWorldSubsystem::ProcessTransitionSequence()
{
	CurrentIndex++;
	OnTransitionSequenceEvent.Broadcast();
}

void UTutorialWorldSubsystem::ProcessSequenceEvent()
{
	if (!TutorialManager.IsValid())
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("TutorialManager is not valid. Cannot process tutorial event."));
		return;
	}
	
	if (TutorialSequenceNames[CurrentIndex] == FName("TutorialSequence_012"))
	{
		TutorialManager->ToggleTutorialBGM(false);
	}
	else if (TutorialSequenceNames[CurrentIndex] == FName("TutorialSequence_016"))
	{
		TutorialManager->SpawnInstruments();
		
		if (CachedTromboneGameInstance)
		{
			CachedTromboneGameInstance->SetSelectedSongTag(TromboneGamePlayTags::Trombone_Rhythm_Song_MapT);
		}
		
		if (URhythmSubsystem* Subsystem = GetCachedRhythmSubsystem())
		{
			Subsystem->StartRhythmGame(TromboneGamePlayTags::Trombone_Rhythm_Song_MapT);
		}
	}
	else if (TutorialSequenceNames[CurrentIndex] == FName("TutorialSequence_018"))
	{
		TutorialManager->UnequipMyCharacter();
		TutorialManager->DestroySpawnedInstruments();
	}
	else if (TutorialSequenceNames[CurrentIndex] == FName("TutorialSequence_021"))
	{
		TutorialManager->SpawnDummyCharacterWithInstrument(EWeaponType::Trombone);
	}
	else if (TutorialSequenceNames[CurrentIndex] == FName("TutorialSequence_023"))
	{
		TutorialManager->UnequipMyCharacter();
	}
	else if (TutorialSequenceNames[CurrentIndex] == FName("TutorialSequence_025"))
	{
		TutorialManager->UnequipMyCharacter();
		TutorialManager->DestroySpawnedInstruments();
	}
	else if (TutorialSequenceNames[CurrentIndex] == FName("TutorialSequence_030"))
	{
		TutorialManager->SpawnInstruments();
		TutorialManager->ActivateGimmicks();
	}
	else if (TutorialSequenceNames[CurrentIndex] == FName("TutorialSequence_032"))
	{
		TutorialManager->UnequipMyCharacter();
		TutorialManager->DestroySpawnedInstruments();
		TutorialManager->DeactivateGimmicks();
	}
}

void UTutorialWorldSubsystem::ShowExtraData(const ETutorialExtraDataType ExtraDataType, const FString& ExtraDataPath) const
{
	switch (ExtraDataType)
	{
		case ETutorialExtraDataType::Image:
			{
				UTexture2D* Image = LoadObject<UTexture2D>(nullptr, *ExtraDataPath);
				if (Image)
				{
					OnShowExtraDataEvent.Broadcast(Image);
				}
				else
				{
					LOG_WITH_CURRENT_CONTEXT(Warning, FString::Printf(TEXT("Failed to load extra data image from path %s"), *ExtraDataPath));
				}
				break;
			}
		case ETutorialExtraDataType::Video:
			{
				// TODO :Load and play the video
				break;
			}
		default:
			break;
	}
}

void UTutorialWorldSubsystem::HandleOnNoteDetected(const ENoteResult NoteResult)
{
	if (NoteResult == ENoteResult::Excellent)
	{
		ReportAction(EQuestConditionType::GetNoteLevel, EQuestConditionParamType::Specific, FString("Excellent"));
	}
}

URhythmSubsystem* UTutorialWorldSubsystem::GetCachedRhythmSubsystem()
{
	if (CachedRhythmSubsystem.IsValid())
	{
		return CachedRhythmSubsystem.Get();
	}
	
	if (const UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (URhythmSubsystem* Subsystem = GI->GetSubsystem<URhythmSubsystem>())
			{
				CachedRhythmSubsystem = Subsystem;
				return CachedRhythmSubsystem.Get();
			}
		}
	}

	LOG_WITH_CURRENT_CONTEXT(Error, TEXT("Failed to get RhythmSubsystem"));
	return nullptr;
}

void UTutorialWorldSubsystem::RegisterManager(ATutorialManager* InManager)
{
	TutorialManager = InManager;
	
	const float TutorialStartDelay = UTromboneConfig::Get()->TutorialStartDelay;
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Tutorial);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_Tutorial, this, &ThisClass::ProcessTutorial, TutorialStartDelay, false);
}

void UTutorialWorldSubsystem::UnregisterManager()
{
	TutorialManager = nullptr;
}

void UTutorialWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	const UTromboneConfig* Config = UTromboneConfig::Get();
    
	check(Config);
	
	if (ensureMsgf(!Config->TutorialDataTable.IsNull(), TEXT("TutorialDataTable is NOT assigned in UTromboneConfig!")))
	{
		TutorialDataTable = Config->TutorialDataTable.LoadSynchronous();
	}
    
	if (ensureMsgf(!Config->QuestDataTable.IsNull(), TEXT("QuestDataTable is NOT assigned in UTromboneConfig!")))
	{
		QuestDataTable = Config->QuestDataTable.LoadSynchronous();
	}
	
	if (TutorialDataTable)
	{
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
	}
	
	if (const UWorld* World = GetWorld())
	{
		CachedTromboneGameInstance = Cast<UTromboneGameInstance>(World->GetGameInstance());
	}
	
	CurrentIndex = 0;
	IntervalAfterQuestCompletion = Config->IntervalAfterQuestCompletion;
	
	if (URhythmSubsystem* Subsystem = GetCachedRhythmSubsystem())
	{
		Subsystem->OnNoteDetected.AddDynamic(this, &ThisClass::HandleOnNoteDetected);
	}
}

void UTutorialWorldSubsystem::Deinitialize()
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerHandle_Tutorial);
	}
	
	OnDialogueSequenceEvent.Clear();
	OnQuestSequenceEvent.Clear();
	OnTransitionSequenceEvent.Clear();
	OnQuestCompletedEvent.Clear();
	OnShowExtraDataEvent.Clear();
	
	Super::Deinitialize();
}

bool UTutorialWorldSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (Super::ShouldCreateSubsystem(Outer))
	{
		const UWorld* World = Cast<UWorld>(Outer);
		return World && World->GetMapName().Contains(TEXT("Tutorial"));
	}
	return false;
}
