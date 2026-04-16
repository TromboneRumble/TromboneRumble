#include "Actors/Tutorial/TutorialManager.h"
#include "AkGameplayStatics.h"
#include "Actors/Tutorial/TutorialDummy.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Framework/TromboneGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Actors/Gimmick/GimmickManager.h"
#include "Subsystems/WorldSubsystem/TutorialWorldSubsystem.h"
#include "UI/UserWidgets/Popup/TwoButtonWithoutClosePopup.h"
#include "Utilities/DebugHelper.h"
#include "Utilities/EnumHelper.h"
#include "Utilities/TromboneStatics.h"

void ATutorialManager::BeginPlay()
{
	Super::BeginPlay();
	
	if (UTutorialWorldSubsystem* Sub = GetWorld()->GetSubsystem<UTutorialWorldSubsystem>())
	{
		Sub->RegisterManager(this);
		Sub->OnDialogueSequenceEvent.AddDynamic(this, &ThisClass::OnTutorialDialogueSequence);
		Sub->OnQuestSequenceEvent.AddDynamic(this, &ThisClass::OnTutorialQuestSequence);
		Sub->OnTransitionSequenceEvent.AddDynamic(this, &ThisClass::OnTutorialTransitionSequence);
	}
	
	ToggleTutorialBGM(true);
}

void ATutorialManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UTutorialWorldSubsystem* Sub = GetWorld()->GetSubsystem<UTutorialWorldSubsystem>())
	{
		Sub->OnDialogueSequenceEvent.RemoveAll(this);
		Sub->OnQuestSequenceEvent.RemoveAll(this);
		Sub->OnTransitionSequenceEvent.RemoveAll(this);
		Sub->UnregisterManager();
	}
	
	Super::EndPlay(EndPlayReason);
}

void ATutorialManager::OnTutorialDialogueSequence(const FText& DialogueString)
{
	TogglePlayerInput(false);
}

void ATutorialManager::OnTutorialQuestSequence(const TArray<FQuestUIData>& QuestUIDataArray)
{
	TogglePlayerInput(true);
}

void ATutorialManager::OnTutorialTransitionSequence()
{
	TogglePlayerInput(false);
}

void ATutorialManager::TogglePlayerInput(bool bIsEnabled)
{
	if (ADefaultTromboneCharacter* MyCharacter = GetCachedPlayerCharacter())
	{
		MyCharacter->SetPlayerInput(bIsEnabled);
	}
}

void ATutorialManager::ToggleTutorialBGM(bool bIsOn)
{
	if (bIsOn)
	{
		if (TutorialBGMEvent)
		{
			UAkGameplayStatics::PostEvent(TutorialBGMEvent, UGameplayStatics::GetPlayerPawn(this,0), 0, FOnAkPostEventCallback());
		}
	}
	else
	{
		if (TutorialBGMOffSwitch)
		{
			UAkGameplayStatics::SetSwitch(TutorialBGMOffSwitch, UGameplayStatics::GetPlayerPawn(this, 0),FName(""),FName(""));
		}
	}
}

void ATutorialManager::UnequipMyCharacter()
{
	if (ADefaultTromboneCharacter* MyCharacter = GetCachedPlayerCharacter())
	{
		MyCharacter->Unequip();
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
		if (AInstrumentBase* Instrument = Cast<AInstrumentBase>(SpawnedInstrument))
		{
			Instrument->SetCanBeSwitched(true);
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

void ATutorialManager::SpawnDummyCharacterWithInstrument(EWeaponType WeaponType)
{
	if (!DummyCharacterClass)
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("DummyCharacterClass is not set"));
		return;
	}
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FRotator Rotation = FRotator(0, 180, 0);
	ATutorialDummy* SpawnedDummy = GetWorld()->SpawnActor<ATutorialDummy>(DummyCharacterClass, DummyCharacterSpawnLocation, Rotation, SpawnParams);
	if (!SpawnedDummy)
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("Failed to spawn dummy character"));
		return;
	}
	
	AInstrumentBase* SpawnedInstrument = SpawnInstrument(WeaponType);
	if (IsValid(SpawnedInstrument))
	{
		SpawnedDummy->Equip(SpawnedInstrument);
	}
}

void ATutorialManager::ActivateGimmicks()
{
	if (AGimmickManager* Manager = GetCachedGimmickManager())
	{
		Manager->ActivateAllGimmicks();
	}
}

void ATutorialManager::DeactivateGimmicks()
{
	if (AGimmickManager* Manager = GetCachedGimmickManager())
	{
		Manager->DeactivateAllGimmicks();
	}
}

void ATutorialManager::ShowTutorialCompletePopup() const
{
	if (const UTromboneGameInstance* GI = Cast<UTromboneGameInstance>(GetGameInstance()))
	{
		const FText Title = GI->GetTutorialUIText(TEXT("StringKey_TutorialEndTitle"));
		const FText Description = GI->GetTutorialUIText(TEXT("StringKey_TutorialEndDescription"));
		const FText LeftButtonText = GI->GetUIText(TEXT("Common_Yes"));
		const FText RightButtonText = GI->GetUIText(TEXT("StringKey_Common_GoToMainMenu"));

		const TFunction<void()> LeftCallback = [this]()
		{
			UTromboneStatics::OpenLevel(GetWorld(), ELevelState::Tutorial);
		};

		const TFunction<void()> RightCallback = [this]()
		{
			UTromboneStatics::OpenLevel(GetWorld(), ELevelState::MainMenu);
		};
		
		
		UTwoButtonWithoutClosePopup* Popup = UTromboneStatics::ShowPopup<UTwoButtonWithoutClosePopup>(GetWorld());
		Popup->OnInit(Title, Description, LeftButtonText, RightButtonText, LeftCallback, RightCallback, false);
	}
}

ADefaultTromboneCharacter* ATutorialManager::GetCachedPlayerCharacter()
{
	if (CachedPlayerCharacter.IsValid())
	{
		return CachedPlayerCharacter.Get();
	}

	if (ACharacter* Character = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
	{
		CachedPlayerCharacter = Cast<ADefaultTromboneCharacter>(Character);
		return CachedPlayerCharacter.Get();
	}
	
	LOG_WITH_CURRENT_CONTEXT(Error, TEXT("Failed to get player character"));
	return nullptr;
}

AGimmickManager* ATutorialManager::GetCachedGimmickManager()
{
	if (CachedGimmickManager.IsValid())
	{
		return CachedGimmickManager.Get();
	}

	if (AGimmickManager* Manager = Cast<AGimmickManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGimmickManager::StaticClass())))
	{
		CachedGimmickManager = Manager;
		return CachedGimmickManager.Get();
	}
	
	LOG_WITH_CURRENT_CONTEXT(Error, TEXT("Failed to get GimmickManager"));
	return nullptr;
}
