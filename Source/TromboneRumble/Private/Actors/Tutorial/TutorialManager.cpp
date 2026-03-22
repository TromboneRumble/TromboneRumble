#include "Actors/Tutorial/TutorialManager.h"
#include "Blueprint/UserWidget.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Data/QuestData.h"
#include "Data/TutorialData.h"
#include "DeveloperSettings/TromboneConfig.h"
#include "Framework/TromboneGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/RhythmSubsystem.h"
#include "UI/UserWidgets/Popup/TwoButtonWithoutClosePopup.h"
#include "Utilities/TromboneStatics.h"

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
	
	if (UGameInstance* GI = GetGameInstance())
	{
		if (URhythmSubsystem* Subsystem = GI->GetSubsystem<URhythmSubsystem>())
		{
			RhythmSubsystem = Subsystem;
			RhythmSubsystem->OnNoteDetected.AddDynamic(this, &ThisClass::HandleOnNoteDetected);
		}
		
		if (UTromboneGameInstance* TGI = Cast<UTromboneGameInstance>(GI))
		{
			TromboneGameInstance = TGI;
		}
	}
	
	if (!RhythmSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("RhythmSubsystem is null"));
	}
}

void ATutorialManager::ReportAction(EQuestConditionType Condition, EQuestConditionParamType ConditionParam_0, FString ConditionParam_1)
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
			bIsQuestSequenceProcessing = false;
			
			FTimerDelegate TimerDelegate = FTimerDelegate::CreateLambda([this]()
			{
				ProcessTutorial();
			});
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_Tutorial, TimerDelegate, IntervalAfterQuestCompletion, false);
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
	
	if (bIsQuestSequenceProcessing) return;
	
	if (CurrentIndex >= TutorialSequenceNames.Num())
	{
		ShowTutorialCompletePopup();
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
}

void ATutorialManager::ProcessDialogueSequence()
{
	const FString DescriptionStringId = TutorialDataTable->FindRow<FTutorialData>(TutorialSequenceNames[CurrentIndex], FString())->DialogueStringID;
	const FText Dialogue = TromboneGameInstance->GetTutorialUIText(DescriptionStringId);
	
	CurrentIndex++;
	ProcessSequenceSideEffect();
	OnDialogueSequence.Broadcast(Dialogue);
}

void ATutorialManager::ProcessQuestSequence()
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
			const FText Description = TromboneGameInstance->GetTutorialUIText(DescriptionStringId);
			
			FFormatNamedArguments FormatArgs = { 
				{ TEXT("1"), FText::FromString(QuestData->QuestConditionParam_1) },
				{ TEXT("0"), QuestData->QuestCondition_Count }
			};
			const FText FormattedDescription = FText::Format(Description, FormatArgs);
			UTexture2D* Icon = LoadObject<UTexture2D>(nullptr, *QuestData->IconPath);
			
			if (!Icon)
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to load icon for Quest ID %s from path %s"), *QuestID, *QuestData->IconPath);
			}
			
			QuestUIDataArray.Add({ QuestID, FormattedDescription, Icon });
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Quest ID %s not found in QuestDataTable"), *QuestID);
		}
	}
	
	CurrentIndex++;
	ProcessSequenceSideEffect();
	OnQuestSequence.Broadcast(QuestUIDataArray);
}

void ATutorialManager::ProcessTransitionSequence()
{
	CurrentIndex++;
	OnTransitionSequence.Broadcast();
}

void ATutorialManager::ProcessSequenceSideEffect()
{
	if (TutorialSequenceNames[CurrentIndex] == FName("TutorialSequence_016"))
	{
		if (UTromboneGameInstance* GI = Cast<UTromboneGameInstance>(GetGameInstance()))
		{
			SpawnInstruments();
			GI->SetSelectedSongTag(TromboneGamePlayTags::Trombone_Rhythm_Song_MapT);
			RhythmSubsystem->StartRhythmGame(TromboneGamePlayTags::Trombone_Rhythm_Song_MapT);
		}
	}
	else if (TutorialSequenceNames[CurrentIndex] == FName("TutorialSequence_017"))
	{
		UE_LOG(LogTemp, Warning, TEXT("CAlled"));
		RhythmSubsystem->PauseRhythmGame();
	}
	else if (TutorialSequenceNames[CurrentIndex] == FName("TutorialSequence_021"))
	{
		SpawnDummyCharacter();
		
		if (ADefaultTromboneCharacter* MyCharacter = GetPlayerCharacter())
		{
			MyCharacter->Unequip();
		}
	}
	else if (TutorialSequenceNames[CurrentIndex] == FName("TutorialSequence_030"))
	{
		RhythmSubsystem->ResumeRhythmGame();
		
		if (ADefaultTromboneCharacter* MyCharacter = GetPlayerCharacter())
		{
			MyCharacter->Unequip();
		}
	}
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

void ATutorialManager::ShowTutorialCompletePopup()
{
	if (UTromboneGameInstance* GI = Cast<UTromboneGameInstance>(GetGameInstance()))
	{
		const FText Title = GI->GetTutorialUIText(TEXT("StringKey_TutorialEndTitle"));
		const FText Description = GI->GetTutorialUIText(TEXT("StringKey_TutorialEndDescription"));
		const FText LeftButtonText = GI->GetUIText(TEXT("Common_Yes"));
		const FText RightButtonText = GI->GetUIText(TEXT("StringKey_Common_GoToMainMenu"));
		
		FOnPopupAction LeftAction, RightAction;
		LeftAction.AddLambda([this]()
		{
			UTromboneStatics::OpenLevel(GetWorld(), ELevelState::Tutorial);
		});
		RightAction.AddLambda([this]()
		{
			UTromboneStatics::OpenLevel(GetWorld(), ELevelState::MainMenu);
		});
		
		const UTromboneConfig* Config = UTromboneConfig::Get();
		auto* Popup = CreateWidget<UTwoButtonWithoutClosePopup>(GetWorld(), Config->TwoButtonWithoutClosePopupWidgetClass);
		Popup->OnInit(Title, Description, LeftButtonText, RightButtonText, LeftAction, RightAction);
		
		UTromboneStatics::SetInputConfig(GetWorld(), true, true, true);
	}
}

ADefaultTromboneCharacter* ATutorialManager::GetPlayerCharacter() const
{
	if (ACharacter* MyCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
	{
		if (ADefaultTromboneCharacter* PlayerCharacter = Cast<ADefaultTromboneCharacter>(MyCharacter))
		{
			return PlayerCharacter;
		}
	}
	
	UE_LOG(LogTemp, Error, TEXT("Get Player Character Failed"));
	return nullptr;
}

void ATutorialManager::HandleOnNoteDetected(const ENoteResult NoteResult)
{
	if (NoteResult == ENoteResult::Excellent)
	{
		ReportAction(EQuestConditionType::GetNoteLevel, EQuestConditionParamType::Specific, FString("Excellent"));
	}
}

void ATutorialManager::HandleOnSpotlightBonusEarned()
{
	ReportAction(EQuestConditionType::HitSpotlight, EQuestConditionParamType::Any, FString());
}	