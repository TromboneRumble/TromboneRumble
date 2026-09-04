// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Poolable.h"
#include "Utilities/Defines.h"
#include "RhythmNote.generated.h"

class UActorPoolSubsystem;
class ANoteVisualizer;
class ARhythmActor;
class ARhythmNoteSpawner;
class URhythmNoteChannelSubsystem;
class URhythmNoteUIControllerComponent;
class USphereComponent;
struct FTimerHandle;


UCLASS(Abstract)
class TROMBONERUMBLE_API ARhythmNote : public AActor, public IPoolable
{
	GENERATED_BODY()
	
public:	
	ARhythmNote();
	virtual void Tick(float DeltaTime) override;
	void SetPause(bool InPause);

	// IPoolable interface
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnTakenFromPool();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnReturnToPool();
	// End of IPoolable interface

	void InitNote(const ARhythmActor* InRhythmActor, const ARhythmNoteSpawner* InSpawner, const TSubclassOf<ANoteVisualizer>& InJudgementRingClass, float InTimeToComplete, const FString& InUserCueName, double InSpawnMusicTimeSec);
	void SetToShortNote();
	void SetToLongNoteStart();
	void SetToLongNoteEnd();

	UFUNCTION(BlueprintNativeEvent,BlueprintCallable, Category = "Rhythm")
	void MoveNotes();
	void MoveNotes_Implementation();

	void StartSyncDebugTimer(ARhythmActor* RhythmActor, float InDelaySeconds);
	void CancelSyncDebugTimer();

	UPROPERTY()
	FNoteHandle NoteHandle;

	void SpawnRhythmResultWidget(ENoteResult InNoteResult);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	UFUNCTION()
	void OnMusicUserCueHandler(FName CueName);


	// Components
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> OuterSphere = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> InnerSphere = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<URhythmNoteUIControllerComponent> RhythmNoteUIControllerComponent = nullptr;
	// ~ Components

	// Cached References
	UPROPERTY()
	TWeakObjectPtr<URhythmNoteChannelSubsystem> CachedRhythmNoteChannelSubsystem = nullptr;

	UPROPERTY(Transient)
	TWeakObjectPtr<UActorPoolSubsystem> CachedActorPoolSubsystem = nullptr;

	UPROPERTY(Transient)
	TSubclassOf<ANoteVisualizer> CachedNoteVisualizerClass = nullptr;

	UPROPERTY(Transient)
	TWeakObjectPtr<ANoteVisualizer> CachedNoteVisualizer = nullptr;

	UPROPERTY(Transient)
	TWeakObjectPtr<ARhythmActor> CachedRhythmActor = nullptr;

	UPROPERTY(Transient)
	TWeakObjectPtr<ARhythmNoteSpawner> ParentSpawner = nullptr;
	// ~Cached References

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm", meta = (AllowPrivateAccess = "true"))
	float TimeToComplete = 3.f;

	bool bIsMoving = false;

	// 이 노트가 스폰된 순간의 음악 시각(초). 이동 진행도를 여기서부터 잰다
	double SpawnMusicTimeSec = 0.0;

	// 스폰 지점에서 판정선까지의 거리(uu)
	static constexpr float NoteTravelDistance = 1000.f;

	// 이 진행도를 넘으면 Destroyer가 놓친 것으로 보고 강제 회수한다 (판정선 1.5배 지점)
	static constexpr float NoteBackstopAlpha = 1.5f;

	// 판정선 도달 로그를 노트당 한 번만 찍기 위한 플래그
	bool bSyncArrivalLogged = false;

	FVector StartLocation = FVector::Zero();

	FVector EndLocation = FVector::Zero();

	float NoteLifeTime;

	bool bIsLongNote = false;

	bool bIsLongNoteEnd = false;

	EInstrumentType NoteType;

	FTimerHandle SyncDebugTimerHandle;

	bool bHasSyncDebugTimer = false;

public:
	// Getter Setter
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetTimeToComplete(float InTime) { TimeToComplete = FMath::Max(0.f, InTime); }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetIsLongNote(bool IsEnd) { bIsLongNote = IsEnd; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetIsLongNoteEnd(bool IsEnd) { bIsLongNoteEnd = IsEnd; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsLongNote() const { return bIsLongNote; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsLongNoteEnd() const { return bIsLongNoteEnd; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE EInstrumentType GetNoteType() const { return NoteType; }

	FORCEINLINE float GetNoteLifetime() const { return NoteLifeTime; }
	// ~Getter Setter
};
