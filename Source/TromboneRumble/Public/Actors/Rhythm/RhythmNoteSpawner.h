// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utilities/Defines.h"
#include "RhythmNoteSpawner.generated.h"

class URhythmNoteChannelSubsystem;
class UActorPoolSubsystem;
class UAkSwitchValue;
class UAkAudioEvent;
class UAkCallbackInfo;
enum class EAkCallbackType : uint8;
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

	void InitSpawner(EInstrumentType InType, UAkAudioEvent* InNoteEvent,
		UAkSwitchValue* InChangeSwitch, UAkAudioEvent* InFailEvent);

	UFUNCTION()
	void OnAkCallback(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo);
public:

	UPROPERTY()
	float TimeToComplete = 5.f;

	
protected:
	virtual void BeginPlay() override;

private:
	void CreateSpawnWidget(const ARhythmActor* InRhythmActor);
	void SpawnAndMoveNote(const FString& InUserCueName);


private:
	// Components
	UPROPERTY(EditAnywhere, Category = "Rhythm", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USplineComponent> SplineComponent;

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
	// ~WWise Audio

	// Rhythm Note UI
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<URhythmSpawnWidget> RhythmSpawnWidgetClass;

	UPROPERTY(Transient, EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<URhythmSpawnWidget> SpawnWidget = nullptr;
	// ~Rhythm Note UI

	// Cached Reference

	UPROPERTY(Transient)
	TWeakObjectPtr<UActorPoolSubsystem> CachedActorPoolSubsystem = nullptr;

	UPROPERTY(Transient)
	TWeakObjectPtr<URhythmNoteChannelSubsystem> CachedRhythmNoteChannelSubsystem = nullptr;

	// ~Cached Reference
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ARhythmNote> RhythmNoteClass;

	UPROPERTY(BlueprintReadOnly, Category = "Rhythm", meta = (AllowPrivateAccess = "true"))
	EInstrumentType SpawnerType = EInstrumentType::Invalid;
public:
	//getter setter
	UFUNCTION(BlueprintCallable, Category = "Component")
	FORCEINLINE USplineComponent* GetSplineComponent() const { return SplineComponent; }

	UFUNCTION(BlueprintCallable, Category = "Rhythm")
	FORCEINLINE EInstrumentType GetSpawnerType() const { return SpawnerType; }
	UFUNCTION(BlueprintCallable, Category = "Rhythm")
	FORCEINLINE UAkAudioEvent* GetSpawnNoteEvent() const { return SpawnNoteEvent; }
	UFUNCTION(BlueprintCallable, Category = "Rhythm")
	FORCEINLINE UAkSwitchValue* GetChangeSwitch() const { return ChangeSwitch; }
	UFUNCTION(BlueprintCallable, Category = "Rhythm")
	FORCEINLINE UAkAudioEvent* GetFailEvent() const { return FailEvent; }

	URhythmSpawnWidget* GetSpawnWidget() const { return SpawnWidget; }
};
