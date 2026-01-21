// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RhythmLeaderBoard.generated.h"

class UEasySessionSubsystem;
class UCommonButtonBase;
class ADefaultPlayerState;
class UCanvasPanel;
class URhythmLeaderBoardEntry;
class AInGameState;
class APlayerState;

/**
 * 
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API URhythmLeaderBoard : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	void SetButtonsVisibility(bool bIsVisible);

	UPROPERTY(EditAnywhere, Category = "Leaderboard")
	TSubclassOf<URhythmLeaderBoardEntry> EntryClass;

protected:
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* Canvas_LeaderBoard;

	UFUNCTION()
	void RefreshLeaderboard(APlayerState* AffectedPlayerState);

	// 네트워크 상황에서 로컬 플레이어의 PlayerState는 늦게 할당될 수 있음
	UFUNCTION()
	void HandleLocalPlayerStateChanged(APlayerState* NewPlayerState);
	
	UFUNCTION()
	void HandleExitButtonClicked();
	
	UFUNCTION()
	void OnDestroySessionSuccess();
	
	UFUNCTION()
	void OnDestroySessionFailure();


private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Exit;
	
	UPROPERTY(Transient)
	TObjectPtr<UEasySessionSubsystem> SessionsSubsystem;
	
	UPROPERTY()
	TMap<TWeakObjectPtr<APlayerState>, URhythmLeaderBoardEntry*> EntryMap;

	UPROPERTY(Transient)
	TWeakObjectPtr<ADefaultPlayerState> LocalPlayerState;

	float RowHeight = 55.f;
	int32 MaxVisibleRows = 4;

	void UpdateRowHeight();
};
