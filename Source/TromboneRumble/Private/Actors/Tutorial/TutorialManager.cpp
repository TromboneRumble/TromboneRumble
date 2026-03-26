#include "Actors/Tutorial/TutorialManager.h"
#include "AkGameplayStatics.h"
#include "Actors/Tutorial/TutorialDummy.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Data/QuestData.h"
#include "Data/TutorialData.h"
#include "Framework/TromboneGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/RhythmSubsystem.h"
#include "Actors/Gimmick/GimmickManager.h"
#include "Kismet/KismetInternationalizationLibrary.h"
#include "UI/UserWidgets/Popup/TwoButtonWithoutClosePopup.h"
#include "Utilities/DebugHelper.h"
#include "Utilities/EnumHelper.h"
#include "Utilities/TromboneStatics.h"

ATutorialManager::ATutorialManager()
{
}

void ATutorialManager::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeTutorial();
	
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Tutorial);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_Tutorial, this, &ThisClass::ProcessTutorial, 1.0f, false);
	
	if (UGameInstance* GI = GetGameInstance())
	{
		if (URhythmSubsystem* Subsystem = GI->GetSubsystem<URhythmSubsystem>())
		{
			RhythmSubsystem = Subsystem;
			RhythmSubsystem->OnNoteDetected.AddDynamic(this, &ThisClass::HandleOnNoteDetected);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("RhythmSubsystem is null"));
		}
		
		if (UTromboneGameInstance* TGI = Cast<UTromboneGameInstance>(GI))
		{
			TromboneGameInstance = TGI;
		}
	}
	
	if (TutorialBGMEvent)
	{
		UAkGameplayStatics::PostEvent(TutorialBGMEvent, UGameplayStatics::GetPlayerPawn(this,0), 0, FOnAkPostEventCallback());
	}

	GimmickManager = Cast<AGimmickManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGimmickManager::StaticClass()));

}

void ATutorialManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerHandle_Tutorial);
	}
	
	OnDialogueSequence.Clear();
	OnQuestSequence.Clear();
	OnTransitionSequence.Clear();
	OnShowExtraData.Clear();
	
	Super::EndPlay(EndPlayReason);
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
			
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Tutorial);
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_Tutorial, this, &ThisClass::ProcessTutorial, IntervalAfterQuestCompletion, false);
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
	if (ADefaultTromboneCharacter* MyCharacter = GetPlayerCharacter())
	{
		MyCharacter->SetPlayerInput(false);
	}
	
	if (const FTutorialData* TutorialData = TutorialDataTable->FindRow<FTutorialData>(TutorialSequenceNames[CurrentIndex], FString()))
	{
		const FString DescriptionStringId = TutorialData->DialogueStringID;
		const FText Dialogue = TromboneGameInstance->GetTutorialUIText(DescriptionStringId);
		
		ProcessSequenceSideEffect();
		CurrentIndex++;
		OnDialogueSequence.Broadcast(Dialogue);
			
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
			UE_LOG(LogTemp, Warning, TEXT("Unsupported culture %s. Defaulting to English extra data path."), *CurrentCulture);
			ExtraDataPath = TutorialData->ExtraDataPath_en;
		}
			
		ShowExtraData(ExtraDataType, ExtraDataPath);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Tutorial data not found for sequence %s"), *TutorialSequenceNames[CurrentIndex].ToString());
	}

}

void ATutorialManager::ProcessQuestSequence()
{
	if (ADefaultTromboneCharacter* MyCharacter = GetPlayerCharacter())
	{
		MyCharacter->SetPlayerInput(true);
	}
	
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
	
	ProcessSequenceSideEffect();
	CurrentIndex++;
	OnQuestSequence.Broadcast(QuestUIDataArray);
}

void ATutorialManager::ProcessTransitionSequence()
{
	if (ADefaultTromboneCharacter* MyCharacter = GetPlayerCharacter())
	{
		MyCharacter->SetPlayerInput(false);
	}
	
	CurrentIndex++;
	OnTransitionSequence.Broadcast();
}

void ATutorialManager::ProcessSequenceSideEffect()
{
	if (TutorialSequenceNames[CurrentIndex] == FName("TutorialSequence_012"))
	{
		if (TutorialBGMOffSwitch)
		{
			UAkGameplayStatics::SetSwitch(TutorialBGMOffSwitch, UGameplayStatics::GetPlayerPawn(this, 0),FName(""),FName(""));
		}
	}
	else if (TutorialSequenceNames[CurrentIndex] == FName("TutorialSequence_016"))
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
		RhythmSubsystem->PauseRhythmGame();
	}
	else if (TutorialSequenceNames[CurrentIndex] == FName("TutorialSequence_021"))
	{
		if (ADefaultTromboneCharacter* MyCharacter = GetPlayerCharacter())
		{
			MyCharacter->Unequip();
		}
		DestroySpawnedInstruments();
		ATutorialDummy* SpawnedDummy = SpawnDummyCharacter();
		AInstrumentBase* SpawnedInstrument = SpawnInstrument(EWeaponType::Trombone);
		if (IsValid(SpawnedInstrument))
		{
			SpawnedDummy->Equip(SpawnedInstrument);
		}
	}
	else if (TutorialSequenceNames[CurrentIndex] == FName("TutorialSequence_023"))
	{
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
		if (GimmickManager.IsValid() && GimmickManager.Get())
		{
			GimmickManager->ActivateGimmickByType(EGimmickType::Spotlight);
			GimmickManager->ActivateGimmickByType(EGimmickType::Puddle);
			GimmickManager->ActivateGimmickByType(EGimmickType::Trash);
		}

	}
}

void ATutorialManager::ShowExtraData(ETutorialExtraDataType ExtraDataType, const FString& ExtraDataPath)
{
	switch (ExtraDataType)
	{
		case ETutorialExtraDataType::Image:
		{
			UTexture2D* Image = LoadObject<UTexture2D>(nullptr, *ExtraDataPath);
			if (Image)
			{
				OnShowExtraData.Broadcast(Image);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to load extra data image from path %s"), *ExtraDataPath);
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
		
		AActor* SpawnedInstrument = GetWorld()->SpawnActor<AActor>(WeaponClass, SpawnLocation, SpawnRotation, SpawnParams);
		if (!SpawnedInstrument)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to spawn instrument of class %s"), *WeaponClass->GetName());
		}
		
		SpawnedInstruments.Add(SpawnedInstrument);
	}
}

AInstrumentBase* ATutorialManager::SpawnInstrument(EWeaponType WeaponType)
{
	if (WeaponType == EWeaponType::Invalid)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid weapon type provided for spawning instrument"));
		return nullptr;
	}
	
	if (!WeaponClasses.Contains(WeaponType))
	{
		UE_LOG(LogTemp, Warning, TEXT("No weapon class found for weapon type %s"), *EnumHelper::EnumToString(WeaponType));
		return nullptr;
	}

	const TSubclassOf<AActor> WeaponClass = WeaponClasses[WeaponType];
	if (!WeaponClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon class for weapon type %s is null"), *EnumHelper::EnumToString(WeaponType));
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FVector SpawnLocation = WeaponSpawnLocations.Contains(WeaponType) ? WeaponSpawnLocations[WeaponType] : FVector::ZeroVector;
	const FRotator SpawnRotation = FRotator::ZeroRotator;

	if (AActor* SpawnedInstrument = GetWorld()->SpawnActor<AActor>(WeaponClass, SpawnLocation, SpawnRotation, SpawnParams))
	{
		SpawnedInstruments.Add(SpawnedInstrument);
		return Cast<AInstrumentBase>(SpawnedInstrument);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Failed to spawn instrument of class %s"), *WeaponClass->GetName());
	return nullptr;
}

void ATutorialManager::DestroySpawnedInstruments()
{
	for (int32 i = SpawnedInstruments.Num() - 1; i >= 0; --i)
	{
		AActor* Instrument = SpawnedInstruments[i].Get();
		if (IsValid(Instrument))
		{
			Instrument->Destroy();
		}
	}
	SpawnedInstruments.Empty();
}

ATutorialDummy* ATutorialManager::SpawnDummyCharacter()
{
	if (!DummyCharacterClass)
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("DummyCharacterClass is not set"));
		return nullptr;
	}
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FRotator Rotation = FRotator(0, 180, 0);
	if (ATutorialDummy* SpawnedDummy = GetWorld()->SpawnActor<ATutorialDummy>(DummyCharacterClass, DummyCharacterSpawnLocation, Rotation, SpawnParams))
	{
		if (SpawnedDummy)
		{
			SpawnedDummyCharacter = SpawnedDummy;
			return SpawnedDummy;
		}
	}
	
	LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("Failed to spawn dummy character"));
	return nullptr;
}

void ATutorialManager::DestroySpawnedDummyCharacter()
{
	if (SpawnedDummyCharacter)
	{
		SpawnedDummyCharacter->Destroy();
		SpawnedDummyCharacter = nullptr;
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
		
		UTwoButtonWithoutClosePopup* Popup = UTromboneStatics::ShowTwoButtonPopup(GetWorld());
		Popup->OnInit(Title, Description, LeftButtonText, RightButtonText, LeftAction, RightAction, false);
		
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