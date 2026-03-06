// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Utilities/Defines.h"
#include "DefaultPlayerState.generated.h"

class AWeaponBase;
class URhythmSubsystem;
class AInGameState;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLocalScoreChanged, APlayerState*, PlayerState, int32, AddedAmount, EScoreType, ScoreType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComboChanged, ENoteResult, InNoteResult, int32, ComboCount);

USTRUCT(BlueprintType)
struct FRumbleScoreData
{
    GENERATED_BODY()

public:
    // --- 리듬 판정 관련 ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score|Rhythm")
    float TotalScore = 0; 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score|Rhythm")
    int32 PerfectCount = 0; // Perfect 맞춘 개수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score|Rhythm")
    int32 GoodCount = 0;    // Good 맞춘 개수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score|Rhythm")
    int32 MissCount = 0;    // Miss한 횟수

    // --- 악기별 버프 점수 ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score|Buff")
    float TromboneComboBuffScore = 0.0f; // 트럼본으로 얻은 콤보 버프 점수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score|Buff")
    float ViolinBuffScore = 0.0f;        // 바이올린으로 얻은 버프 점수

    // --- 공격 및 인터랙션 점수 ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score|Combat")
    float AttackScore = 0.0f;            // 공격 점수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score|Combat")
    int32 HitCount = 0;                  // 타격 횟수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score|Combat")
    float CymbalsAttackScore = 0.0f;     // 심벌즈로 얻은 공격 점수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score|Combat")
    int32 InstrumentStealCount = 0;      // 악기 스틸 횟수

    // --- 특수 및 기타 점수 ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score|Special")
    int32 SpotlightPickupCount = 0;      // Spotlight 점수를 먹은 횟수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score|Special")
    float OtherScore = 0.0f;             // 기타 점수

    // --- 최종 합산 점수 ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score|Total")
    float TotalPerformanceScore = 0.0f;  // 총 연주 점수

    // 데이터를 초기화하는 편의 함수
    void Reset()
    {
        *this = FRumbleScoreData();
    }
};

UCLASS()
class TROMBONERUMBLE_API ADefaultPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ADefaultPlayerState();

    virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_PlayerName() override;
	virtual void OnRep_Score() override;
	virtual void CopyProperties(APlayerState* PlayerState) override;	


	// 클라이언트 PlayerState 에서 점수가 바뀌면 GameState에 알림.
	UPROPERTY(BlueprintAssignable)
	FOnLocalScoreChanged OnLocalScoreChanged;

	UPROPERTY(BlueprintAssignable)
	FOnComboChanged OnComboChanged;

	void AddScore(int32 Amount, EScoreType ScoreType);
	UFUNCTION(Server, Reliable)
	void Server_AddScore(int32 Amount, EScoreType ScoreType);

	void HandleCombo(ENoteResult InResult);

	UPROPERTY(VisibleInstanceOnly, Replicated)
	TSubclassOf<AWeaponBase> EquippedWeaponClass;
	
protected:
    FTimerHandle TimerHandle_BindGameState;
    void TryBindGameState();

    UFUNCTION()
    void HandleRhythmGameStateChanged(ERhythmGameState NewState);

    UFUNCTION()
    void HandleNoteDetected(ENoteResult NoteResult);

    UFUNCTION()
    void HandleInGameStateChanged(EInGameState InGameState);

    UFUNCTION()
    void HandleOnInstrumentPicked(EInstrumentType PrevType, EInstrumentType NewType);

	UPROPERTY(ReplicatedUsing = OnRep_SkinColor)
	FLinearColor SkinColor = FLinearColor::Black;

	UFUNCTION()
	void OnRep_SkinColor();

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentCombo = 0;

    UPROPERTY(BlueprintReadOnly, Category = "PlayerState")
    FRumbleScoreData CurrentScoreData;

public:
	//getter setter
	FORCEINLINE float GetRhythmScore() const { return GetScore(); }
	void SetSkinColor(const FLinearColor& InSkinColor);
	FORCEINLINE FLinearColor GetSkinColor() const { return SkinColor; }
	FORCEINLINE int32 GetCurrentCombo() const { return CurrentCombo; }
    FORCEINLINE FRumbleScoreData GetScoreData() const { return CurrentScoreData; }
	// ~getter setter
};
