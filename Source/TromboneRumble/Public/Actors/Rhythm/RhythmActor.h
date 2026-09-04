// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AkGameplayTypes.h"
#include "GameplayTagContainer.h"
#include "TromboneGamePlayTags.h"
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
class UMaterialInterface;


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

	UFUNCTION(BlueprintCallable)
	void PrepareAndStartRhythmGame(const FGameplayTag& InGamePlayTag);

	UFUNCTION(BlueprintCallable)
	void PauseRhythmGame();

	UFUNCTION(BlueprintCallable)
	void ResumeRhythmGame();

	UFUNCTION(BlueprintCallable)
	void StopRhythmGame();
	// ~ Init Game

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void CleanupRhythmGame();

	void PrepareRhythmGame(const FGameplayTag& InGamePlayTag);

	void StartRhythmGame();

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
	void SpawnRhythmRootUI();

private:
	// Rhythm Game Init
	void InitGameState();
	FTimerHandle GameStateInitTimerHandle;

	UPROPERTY()
	bool bIsDataLoaded = false;

	UPROPERTY()
	FGameplayTag LoadedGameplayTag = FGameplayTag::EmptyTag;

	UPROPERTY()
	bool bIsLoadingData = false;     

	UPROPERTY()
	bool bStartRequested = false;

	ARhythmNoteSpawner* GetOrCreateSpawner(EInstrumentType InType);
	bool DestroySpawner(EInstrumentType InType);

	UFUNCTION()
	void OnInstrumentPickedHandler(EInstrumentType PrevType, EInstrumentType NewType);

	UFUNCTION()
	void OnNoteDetectedHandler(ENoteResult InNoteResult);

	UFUNCTION()
	void HandleInGameStateChanged(EInGameState InGameState);

	UPROPERTY()
	bool bAreOtherPlayersReady = false;

	UFUNCTION()
	void WaitForOtherPlayers();
	FTimerHandle CheckPlayersTimerHandle;

	// 곡 데이터 로드에 실패하면 재시도한다. 재시도가 없으면 그 머신만 영영 시작하지 못한다
	UFUNCTION()
	void RetryPrepareRhythmGame();
	FTimerHandle PrepareRetryTimerHandle;
	int32 PrepareRetryCount = 0;

	// 곡 태그의 원본은 GameInstance다. 로드에 실패하면 LoadedGameplayTag는 비어 있다
	FGameplayTag GetSelectedSongTagFromGameInstance() const;

	UFUNCTION()
	void PlayMusic();
	FTimerHandle PlayBackgroundMusicTimerHandle;

	// BGM 포스트가 실패하면 EndOfEvent가 안 와서 곡이 끝난 걸 아무도 모른다
	int32 BGMPostRetryCount = 0;

	// 음악 클럭이 살아있는 스포너를 찾는다. 조회 자체가 모든 스포너의 클럭을 갱신한다
	ARhythmNoteSpawner* GetMasterClockSpawner();

	// BGM 시작 대기 상태. 노트 트랙 클럭이 BGMTriggerTimeSec를 넘으면 재생한다
	bool bWaitingToStartBGM = false;
	double BGMTriggerTimeSec = 3.0;
	UFUNCTION()
	void HandleBGMCallbacks(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo);
	bool bHasReceivedMusicStartCallback = false;
	bool bHasReceivedDurationCallback = false;
	bool bHasShotBGMDelegate = false;
	// ~Rhythm Game Init

	// Note Detection Logic
	ENoteResult ReturnNoteResult(const ARhythmNote* InNote, const TMap<ARhythmNote*, TSet<UPrimitiveComponent*>>& InNoteToHitComps) const;
	ARhythmNote* GetBestNoteFromLineTrace(TMap<ARhythmNote*, TSet<UPrimitiveComponent*>>& InOutNoteToHitComps);
	
	UFUNCTION()
	void OnRhythmDestroyBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	//곡이 시작되고 플레이어가 연타하는것을 막기 위해 이벤트로 제어
	bool bCanDetectNotes = false;
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

	UPROPERTY()
	EInstrumentType FocusedType = EInstrumentType::Background;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 BGMPlayingID = 0;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bIsSensingLongNote = false;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"))
	float RhythmDestroyerBoxExtent = 30.f;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	bool bIsSyncTesting = false;
	// ~Rhythm Game

	// Map Materials
	// 맵별 리듬게임 캐릭터 링 히트박스 머티리얼. 미설정 시 캐릭터 BP에 지정된 기본 머티리얼 사용
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rhythm|Materials", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> HitBoxRingMaterial = nullptr;

	// 맵별 리듬게임 노트 머티리얼. 미설정 시 NoteVisualizer BP에 지정된 기본 머티리얼 사용
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rhythm|Materials", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> NoteVisualizerRingMaterial = nullptr;
	// ~Map Materials

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

	UFUNCTION(BlueprintCallable, Category = "Rhythm")
	FORCEINLINE int32 GetBGMPlayingID() const { return BGMPlayingID; }

	UFUNCTION(BlueprintCallable, Category = "Rhythm|Materials")
	FORCEINLINE UMaterialInterface* GetHitBoxRingMaterial() const { return HitBoxRingMaterial; }

	UFUNCTION(BlueprintCallable, Category = "Rhythm|Materials")
	FORCEINLINE UMaterialInterface* GetNoteVisualizerRingMaterial() const { return NoteVisualizerRingMaterial; }
};
