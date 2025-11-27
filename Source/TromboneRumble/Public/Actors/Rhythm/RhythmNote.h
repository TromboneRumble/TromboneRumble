// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Poolable.h"
#include "Utilities/Defines.h"
#include "RhythmNote.generated.h"

class URhythmSpawnWidget;
class URhythmNoteWidget;
class URhythmNoteChannelSubsystem;
class URhythmNoteUIControllerComponent;
class USplineComponent;
class USphereComponent;
class UTimelineComponent;
class ARhythmNoteSpawner;


UCLASS(Abstract)
class TROMBONERUMBLE_API ARhythmNote : public AActor, public IPoolable
{
	GENERATED_BODY()
	
public:	
	ARhythmNote();
	virtual void Tick(float DeltaTime) override;

	// IPoolable interface
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnTakenFromPool();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnReturnToPool();
	// End of IPoolable interface

	void InitNote(const ARhythmNoteSpawner* InSpawner, URhythmNoteWidget* InNoteWidget, float InTimeToComplete, int32 InLineNum);
	void SetToShortNote();
	void SetToLongNoteStart();
	void SetToLongNoteEnd();

	UFUNCTION(BlueprintNativeEvent,BlueprintCallable, Category = "Rhythm")
	void MoveNotes();
	void MoveNotes_Implementation();

	UPROPERTY()
	FNoteHandle NoteHandle;

	void SpawnRhythmResultWidget(ENoteResult InNoteResult);

protected:
	virtual void BeginPlay() override;

private:

	// Components
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> OuterSphere = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> InnerSphere = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<URhythmNoteUIControllerComponent> RhythmNoteUIControllerComponent = nullptr;
	// ~ Components

	// Cached References
	UPROPERTY(BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<USplineComponent> CachedSplineComponent;

	UPROPERTY()
	TWeakObjectPtr<URhythmNoteChannelSubsystem> CachedRhythmNoteChannelSubsystem = nullptr;
	// ~Cached References

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm", meta = (AllowPrivateAccess = "true"))
	float TimeToComplete = 5.f;

	bool bIsMoving = false;

	FVector StartLocation = FVector::Zero();

	FVector EndLocation = FVector::Zero();

	float NoteLifeTime;

	bool bIsLongNote = false;

	bool bIsLongNoteEnd = false;

	

	EInstrumentType NoteType;

	UPROPERTY(BlueprintReadWrite, Category = "Rhythm", meta = (AllowPrivateAccess = "true"))
	float NoteAlphaOnSpline;
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
