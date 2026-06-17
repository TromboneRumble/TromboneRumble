#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Data/CustomizationSaveData.h"
#include "Utilities/Defines.h"
#include "DefaultPlayerState.generated.h"

class AWeaponBase;
class URhythmSubsystem;
class AInGameState;

/**
 * Delegate triggered when the player's name changes.
 * @param PlayerName The new player name.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerNameChanged, const FString&, PlayerName);

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
    int32 ExcellentCount = 0; // Excellent 맞춘 개수

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
	
#pragma region Events
	
	/** Event when the player's name changes. */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerNameChanged OnPlayerNameChanged;

	// 클라이언트 PlayerState 에서 점수가 바뀌면 GameState에 알림.
	UPROPERTY(BlueprintAssignable)
	FOnLocalScoreChanged OnLocalScoreChanged;

	UPROPERTY(BlueprintAssignable)
	FOnComboChanged OnComboChanged;
	
#pragma endregion

#pragma region Character Skin Color
	
public:
	
	void SetSkinColor(const FLinearColor& InSkinColor);
	
	FLinearColor GetSkinColor() const { return SkinColor; }
	
	UPROPERTY(ReplicatedUsing = OnRep_SkinColor)
	FLinearColor SkinColor = FLinearColor::Black;
	
private:

	UFUNCTION()
	void OnRep_SkinColor();
	
#pragma endregion
	
#pragma region Gameplay
	
public:
	
	void AddScore(int32 Amount, EScoreType ScoreType);
	
	void HandleCombo(ENoteResult InResult);
	
	float GetRhythmScore() const { return GetScore(); }
	
	int32 GetCurrentCombo() const { return CurrentCombo; }
	
	FRumbleScoreData GetScoreData() const { return CurrentScoreData; }
	
protected:

	UPROPERTY(ReplicatedUsing = OnRep_CustomizationData)
	FCustomizationSaveData CustomizationData;

	UFUNCTION()
	void OnRep_CustomizationData();

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentCombo = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PlayerState")
	FRumbleScoreData CurrentScoreData;
	
private:
	
	UFUNCTION(Server, Reliable)
	void Server_AddScore(int32 Amount, EScoreType ScoreType);
	
	UFUNCTION()
	void HandleRhythmGameStateChanged(ERhythmGameState NewState);

	UFUNCTION()
	void HandleNoteDetected(ENoteResult NoteResult);

	UFUNCTION()
	void HandleInGameStateChanged(EInGameState InGameState);

	UFUNCTION()
	void HandleOnInstrumentPicked(EInstrumentType PrevType, EInstrumentType NewType);
	
	void TryBindGameState();
	
	FTimerHandle TimerHandle_BindGameState;
	
#pragma endregion
	
#pragma region Voice
	
public:
	
	UPROPERTY(ReplicatedUsing = OnRep_VoiceSendVolume)
	float VoiceSendVolume = 1.0f;

	UFUNCTION()
	void OnRep_VoiceSendVolume();

	UFUNCTION(Server, Reliable)
	void Server_SetVoiceSendVolume(float Volume);
	
#pragma endregion

	UFUNCTION(Server, Reliable)
	void Server_SetCustomization(FCustomizationSaveData InData);

	// ~ Begin Getter & Setter
	FORCEINLINE FCustomizationSaveData GetCustomizationData() const { return CustomizationData; }
	
public:
	
	// ~ Begin APlayerState Interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_PlayerName() override;
	virtual void OnRep_Score() override;
	// ~ End APlayerState Interface
	
protected:
	
	// ~ Begin APlayerState Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void CopyProperties(APlayerState* PlayerState) override;	
	// ~ End APlayerState Interface
	
public:
	
	bool IsHost() const;
	
	UPROPERTY(VisibleInstanceOnly, Replicated)
	TSubclassOf<AWeaponBase> EquippedWeaponClass;
	
};
