// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utilities/Defines.h"
#include "RhythmActor.generated.h"

class ARhythmNote;
class ARhythmNoteSpawner;
class UBoxComponent;

USTRUCT()
struct FMyStruct
{
	GENERATED_BODY()
	
};

UCLASS()
class TROMBONERUMBLE_API ARhythmActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ARhythmActor();
	virtual void Tick(float DeltaTime) override;


	UFUNCTION(BlueprintCallable)
	void DetectNotes();

	UFUNCTION(BlueprintCallable)
	void DetectLongNoteEnd();

protected:
	virtual void BeginPlay() override;

private:
	FRhythmTraceResult ReturnNoteResult(ARhythmNote* InNote, const TMap<ARhythmNote*, TSet<UPrimitiveComponent*>>& InNoteToHitComps);
	ARhythmNote* GetBestNoteFromLineTrace(TMap<ARhythmNote*, TSet<UPrimitiveComponent*>>& InOutNoteToHitComps);


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> TraceStartPoint = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> TraceEndPoint = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> RhythmNoteDestroyer = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UChildActorComponent> RhythmNoteSpawner = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool IsSensingLongNote = false;

};
