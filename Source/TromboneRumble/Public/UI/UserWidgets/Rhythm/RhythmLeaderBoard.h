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


private:
	UPROPERTY()
	TMap<TWeakObjectPtr<APlayerState>, URhythmLeaderBoardEntry*> EntryMap;

	UPROPERTY(Transient)
	TWeakObjectPtr<ADefaultPlayerState> LocalPlayerState;

	float RowHeight = 75.f;
	int32 MaxVisibleRows = 4;

	void UpdateRowHeight();
};
