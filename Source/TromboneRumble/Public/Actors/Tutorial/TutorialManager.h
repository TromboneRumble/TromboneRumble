#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Items/InstrumentBase.h"
#include "TutorialManager.generated.h"

struct FQuestUIData;
class AGimmickManager;
class UAkSwitchValue;
class ATutorialDummy;
class ADefaultTromboneCharacter;
enum class EWeaponType : uint8;

UCLASS()
class TROMBONERUMBLE_API ATutorialManager : public AActor
{
	GENERATED_BODY()
	
public:
	
	/** Spawn All instruments */
	void SpawnInstruments();
	
	void ShowTutorialCompletePopup() const;
	
	void ToggleTutorialBGM(bool bIsOn);
	
	void UnequipMyCharacter();
	
	void DestroySpawnedInstruments();
	
	void SpawnDummyCharacterWithInstrument(EWeaponType WeaponType);
	
	void ActivateGimmicks();
	
	void DeactivateGimmicks();
	
protected:
	
	/** Spawned instrument actors during the tutorial, used for destroy */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> SpawnedInstruments;
	
	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
	TMap<EWeaponType, FVector> WeaponSpawnLocations;
	
	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
	TSubclassOf<AActor> DummyCharacterClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
	FVector DummyCharacterSpawnLocation;

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
	TObjectPtr<UAkAudioEvent> TutorialBGMEvent = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
	TObjectPtr<UAkSwitchValue> TutorialBGMOffSwitch = nullptr;
	
private:
	
	UFUNCTION()
	void OnTutorialDialogueSequence(const FText& DialogueString);
	
	UFUNCTION()
	void OnTutorialQuestSequence(const TArray<FQuestUIData>& QuestUIDataArray);
	
	UFUNCTION()
	void OnTutorialTransitionSequence();
	
	/** Spawn specific instrument */
	AInstrumentBase* SpawnInstrument(EWeaponType WeaponType);
	
	/** Toggle player input on/off */
	void TogglePlayerInput(bool bIsEnabled);
	
private:
	
	/** @return My character */
	ADefaultTromboneCharacter* GetCachedPlayerCharacter();
	
	/** @return GimmickManager */
	AGimmickManager* GetCachedGimmickManager();
	
	TWeakObjectPtr<ADefaultTromboneCharacter> CachedPlayerCharacter = nullptr;
	
	TWeakObjectPtr<AGimmickManager> CachedGimmickManager = nullptr;
	
protected:
	
	// ~ Begin Actor Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// ~ End Actor Interface
};