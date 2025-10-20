// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RhythmNote.generated.h"

class USplineComponent;
class USphereComponent;
class UTimelineComponent;
class ARhythmNoteSpawner;

UCLASS(Abstract)
class TROMBONERUMBLE_API ARhythmNote : public AActor
{
	GENERATED_BODY()
	
public:	
	ARhythmNote();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintNativeEvent,BlueprintCallable, Category = "Rhythm")
	void MoveNotes();
	void MoveNotes_Implementation();

	float NoteLifeTime;

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

protected:
	virtual void BeginPlay() override;

private:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> OuterSphere = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> MiddleSphere = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> InnerSphere = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm", meta = (AllowPrivateAccess = "true"))
	float TimeToComplete = 5.f;

	UPROPERTY(BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<ARhythmNoteSpawner> CachedSpawner;

	UPROPERTY(BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<USplineComponent> CachedSplineComponent;

	bool bIsLongNote = false;

	bool bIsLongNoteEnd = false;
};
