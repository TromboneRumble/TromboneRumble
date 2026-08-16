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

	void StopRhythmGame();

	void RemoveActiveNote(ARhythmNote* Note);

	// 노트 트랙(무음)의 재생 위치를 단조 증가 클럭으로 감싼 값(초).
	// 호출하는 것만으로 클럭이 갱신되며, Wwise 조회는 프레임당 1회만 한다.
	double GetMusicTimeSeconds();

	FORCEINLINE bool HasValidMusicClock() const { return bMusicClockValid; }

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
	
	UPROPERTY(EditDefaultsOnly, Category = "Rhythm")
	int32 PoolPrewarmCount = 24;

private:
	void SpawnAndMoveNote(const FString& InUserCueName, double InSpawnMusicTimeSec);

	void ResetMusicClock();

	// 음악 클럭 상태
	double MusicClockSec = 0.0;		// 단조 증가 클럭 (초)
	int32 LastRawPositionMs = 0;	// 직전 조회값. 역행/0-채움을 걸러낸다
	bool bMusicClockValid = false;
	bool bClockPaused = false;
	uint64 LastClockQueryFrame = 0;
	double ClockStallSeconds = 0.0;

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
