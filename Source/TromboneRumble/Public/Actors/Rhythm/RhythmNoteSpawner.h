// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Actors/Rhythm/RhythmActor.h"
#include "Utilities/Defines.h"
#include "RhythmNoteSpawner.generated.h"

class ANoteVisualizer;
class URhythmSubsystem;
class URhythmNoteChannelSubsystem;
class UActorPoolSubsystem;
class UAkSwitchValue;
class UAkAudioEvent;
class UAkCallbackInfo;
enum class EAkCallbackType : uint8;
class ARhythmActor;
class UArrowComponent;
class ARhythmNote;
class URhythmSpawnWidgetBase;

UCLASS(Abstract)
class TROMBONERUMBLE_API ARhythmNoteSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	ARhythmNoteSpawner();

	void InitSpawner(EInstrumentType InType, UAkAudioEvent* InNoteEvent,
		UAkSwitchValue* InChangeSwitch, UAkAudioEvent* InFailEvent, bool InIsSyncTesting);

	UFUNCTION()
	void OnAkCallback(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo);

	void PauseRhythmGame();

	void ResumeRhythmGame();

	void RemoveActiveNote(ARhythmNote* Note);
public:

	UPROPERTY()
	float TimeToComplete = 3.f;

	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(Transient)
	TSet<TObjectPtr<ARhythmNote>> ActiveNotes;

	// Components
	UPROPERTY(EditAnywhere, Category = "Rhythm", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UArrowComponent> ArrowComponent;
	// ~Components

	// WWise Audio
	UPROPERTY(BlueprintReadOnly, Category = "Rhythm", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAkAudioEvent> SpawnNoteEvent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Rhythm", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAkSwitchValue> ChangeSwitch = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Rhythm", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAkAudioEvent> FailEvent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Rhythm", meta = (AllowPrivateAccess = "true"))
	int32 NoteSpawnPlayingID = 0;
	// ~WWise Audio

	// Cached Reference

	UPROPERTY(Transient)
	TWeakObjectPtr<ARhythmActor> CachedRhythmActor = nullptr;

	UPROPERTY(Transient)
	TWeakObjectPtr<UActorPoolSubsystem> CachedActorPoolSubsystem = nullptr;

	UPROPERTY(Transient)
	TWeakObjectPtr<URhythmNoteChannelSubsystem> CachedRhythmNoteChannelSubsystem = nullptr;

	// ~Cached Reference

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ARhythmNote> RhythmNoteClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ANoteVisualizer> NoteVisualizerClass;

	UPROPERTY(BlueprintReadOnly, Category = "Rhythm", meta = (AllowPrivateAccess = "true"))
	EInstrumentType SpawnerType = EInstrumentType::Invalid;

	UPROPERTY(EditAnywhere, Category = "Rhythm")
	bool IsSyncTesting = false;

private:
	void SpawnAndMoveNote(const FString& InUserCueName);

public:
	//getter setter
	UFUNCTION(BlueprintCallable, Category = "Rhythm")
	FORCEINLINE EInstrumentType GetSpawnerType() const { return SpawnerType; }
	UFUNCTION(BlueprintCallable, Category = "Rhythm")
	FORCEINLINE UAkAudioEvent* GetSpawnNoteEvent() const { return SpawnNoteEvent; }
	UFUNCTION(BlueprintCallable, Category = "Rhythm")
	FORCEINLINE UAkSwitchValue* GetChangeSwitch() const { return ChangeSwitch; }
	UFUNCTION(BlueprintCallable, Category = "Rhythm")
	FORCEINLINE UAkAudioEvent* GetFailEvent() const { return FailEvent; }

	UFUNCTION(BlueprintCallable, Category = "Rhythm")
	FORCEINLINE int32 GetNoteSpawnPlayingID() const { return NoteSpawnPlayingID; }

	FORCEINLINE void SetNoteSpawnPlayingID(int32 InPlayingID) { NoteSpawnPlayingID = InPlayingID; }
};
