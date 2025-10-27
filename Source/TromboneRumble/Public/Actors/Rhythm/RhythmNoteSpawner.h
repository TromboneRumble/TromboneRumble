// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utilities/Defines.h"
#include "RhythmNoteSpawner.generated.h"

class ARhythmActor;
class UArrowComponent;
class USplineComponent;
class ARhythmNote;
class URhythmSpawnWidget;

UCLASS(Abstract)
class TROMBONERUMBLE_API ARhythmNoteSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	ARhythmNoteSpawner();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Rhythm")
	void SpawnRhythmNote(float TimeToComplete = 5.f, bool InIsLongNote = false, bool InIsLongNoteEnd = false);

	UPROPERTY(Transient)
	ARhythmActor* OwnerRhythmActor = nullptr;

	UPROPERTY()
	EInstrumentType SpawnerType = EInstrumentType::Invalid;
protected:
	virtual void BeginPlay() override;

private:
	

	UPROPERTY(EditAnywhere, Category = "Rhythm", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USplineComponent> SplineComponent;

	UPROPERTY(EditAnywhere, Category = "Rhythm", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UArrowComponent> ArrowComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ARhythmNote> RhythmNoteClass;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<URhythmSpawnWidget> RhythmSpawnWidgetClass;

	UPROPERTY(Transient, EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<URhythmSpawnWidget> SpawnWidget = nullptr;

	
public:
	//getter setter
	UFUNCTION(BlueprintCallable, Category = "Component")
	FORCEINLINE USplineComponent* GetSplineComponent() const { return SplineComponent; }
};
