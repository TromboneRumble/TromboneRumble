// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RhythmLeaderBoard.generated.h"

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


private:
	UPROPERTY()
	TMap<TWeakObjectPtr<APlayerState>, URhythmLeaderBoardEntry*> EntryMap;

	UPROPERTY(Transient)
	TWeakObjectPtr<ADefaultPlayerState> LocalPlayerState;

	float RowHeight = 50.f;
	int32 MaxVisibleRows = 4;

	void UpdateRowHeight();
};
