// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utilities/Defines.h"
#include "RhythmActor.generated.h"

class URhythmSubsystem;
class UAkSwitchValue;
class UAkAudioEvent;
class UAkComponent;
class URhythmUIRootWidget;
class UActorPoolSubsystem;
class ARhythmNote;
class ARhythmNoteSpawner;
class UBoxComponent;


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
	ENoteResult DetectLongNoteEnd();
	

	// Init Game

	/// <summary>
	/// 악기 전용 RhythmSpawner를 생성하고 초기화
	/// </summary>
	/// <param name="InType">Instrument Type, BGM도 Instrument Type</param>
	/// <param name="InNoteEvent">리듬게임 정보만 들어있는 미디 파일</param>
	/// <param name="InChangeSwitch">플레이어가 해당 악기를 들었을때 실행해줘야 하는 스위치</param>
	/// <param name="InFailEvent">플레이어가 삑사리를 낼때 나는 방구 이벤트</param>
	UFUNCTION(BlueprintCallable)
	void CreateAndInitRhythmSpawner(EInstrumentType InType, UAkAudioEvent* InNoteEvent, UAkSwitchValue* InChangeSwitch, UAkAudioEvent* InFailEvent);

	/// <summary>
	/// 플레이어가 실제로 듣는 BGM 전용 RhythmSpawner를 생성하고 초기화
	/// </summary>
	/// <param name="InSoundEvent">BGM 소리 이벤트</param>
	/// <param name="InNoneSwitch">플레이어가 아무 악기도 안들고 있을때 실행되어야 하는 스위치</param>
	UFUNCTION(BlueprintCallable)
	void InitBGMEvent(UAkAudioEvent* InSoundEvent, UAkSwitchValue* InNoneSwitch);

	UFUNCTION(BlueprintCallable)
	void StartRhythmGame();

	UFUNCTION(BlueprintCallable)
	void SpawnRhythmRootUI();
	// ~ Init Game

protected:
	virtual void BeginPlay() override;

private:
	// Rhythm Game Init
	void PrepareRhythmGame();
	ARhythmNoteSpawner* GetOrCreateSpawner(EInstrumentType InType);
	bool DestroySpawner(EInstrumentType InType);

	UFUNCTION()
	void OnInstrumentPickedHandler(EInstrumentType InType);

	UFUNCTION()
	void OnNoteDetectedHandler(ENoteResult InNoteResult);
	// ~Rhythm Game Init

	// Note Detection Logic
	ENoteResult ReturnNoteResult(const ARhythmNote* InNote, const TMap<ARhythmNote*, TSet<UPrimitiveComponent*>>& InNoteToHitComps) const;
	ARhythmNote* GetBestNoteFromLineTrace(TMap<ARhythmNote*, TSet<UPrimitiveComponent*>>& InOutNoteToHitComps);
	
	UFUNCTION()
	void OnRhythmDestroyBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
	// ~Note Detection Logic

	// Components
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> TraceStartPoint = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> TraceEndPoint = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> RhythmNoteDestroyer = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAkComponent> NoteSpawnComponent = nullptr;

	UPROPERTY(EditAnywhere, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAkComponent> NoteHearingComponent = nullptr;
	// ~Components

	// Subclasses
	UPROPERTY(EditDefaultsOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ARhythmNoteSpawner> RhythmNoteSpawnerClass = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<URhythmUIRootWidget> RhythmUIRootWidgetClass = nullptr;
	// ~Subclasses

	// Rhythm Game
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TMap<EInstrumentType, TObjectPtr<ARhythmNoteSpawner>> RhythmNoteSpawners;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rhythm", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAkAudioEvent> PlayBGMEvent = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UAkSwitchValue* NoneSwitch = nullptr;

	UFUNCTION()
	void PlayMusic();
	FTimerHandle TimerHandle;

	UPROPERTY()
	EInstrumentType FocusedType = EInstrumentType::Background;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool IsSensingLongNote = false;
	// ~Rhythm Game


	// Cached References
	UActorPoolSubsystem* GetCachedActorPoolSubsystem();
	URhythmSubsystem* GetCachedRhythmSubsystem();
	UPROPERTY(Transient)
	TWeakObjectPtr<UActorPoolSubsystem> CachedActorPoolSubsystem = nullptr;

	UPROPERTY(Transient)
	TWeakObjectPtr<URhythmSubsystem> CachedRhythmSubsystem = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<URhythmUIRootWidget> CachedRhythmUIRootWidget = nullptr;
	// ~Cached References
public:
	//getter setter
	UFUNCTION(BlueprintCallable, Category = "Component")
	FORCEINLINE URhythmUIRootWidget* GetRhythmUIRootWidget() const { return CachedRhythmUIRootWidget; }

	UFUNCTION(BlueprintCallable, Category = "Rhythm")
	FORCEINLINE EInstrumentType GetFocusedInstrumentType() const { return FocusedType; }
};
