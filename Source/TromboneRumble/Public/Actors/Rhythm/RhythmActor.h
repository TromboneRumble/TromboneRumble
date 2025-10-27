// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utilities/Defines.h"
#include "RhythmActor.generated.h"

class URhythmUIRootWidget;
class UActorPoolSubsystem;
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

	UFUNCTION(BlueprintCallable)
	ARhythmNoteSpawner* GetOrCreateSpawner(EInstrumentType InType);

	UFUNCTION(BlueprintCallable)
	bool DestroySpawner(EInstrumentType InType);

protected:
	virtual void BeginPlay() override;

private:
	FRhythmTraceResult ReturnNoteResult(ARhythmNote* InNote, const TMap<ARhythmNote*, TSet<UPrimitiveComponent*>>& InNoteToHitComps);
	ARhythmNote* GetBestNoteFromLineTrace(TMap<ARhythmNote*, TSet<UPrimitiveComponent*>>& InOutNoteToHitComps);
	UActorPoolSubsystem* GetCachedSubsystem();
	UFUNCTION()
	void OnRhythmDestroyBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);


	// Components
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> TraceStartPoint = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> TraceEndPoint = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> RhythmNoteDestroyer = nullptr;
	// ~Components

	// Cached References
	UPROPERTY(Transient)
	TWeakObjectPtr<UActorPoolSubsystem> CachedActorPoolSubsystem = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<URhythmUIRootWidget> CachedRhythmUIRootWidget = nullptr;
	// ~Cached References

	UPROPERTY(EditDefaultsOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<URhythmUIRootWidget> RhythmUIRootWidgetClass = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ARhythmNoteSpawner> RhythmNoteSpawnerClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TMap<EInstrumentType, TObjectPtr<ARhythmNoteSpawner>> RhythmNoteSpawners;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool IsSensingLongNote = false;

public:
	//getter setter
	UFUNCTION(BlueprintCallable, Category = "Component")
	FORCEINLINE URhythmUIRootWidget* GetRhythmUIRootWidget() const { return CachedRhythmUIRootWidget; }
};
