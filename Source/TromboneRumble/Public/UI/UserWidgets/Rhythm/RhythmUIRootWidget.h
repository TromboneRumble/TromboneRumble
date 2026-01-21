// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RhythmUIRootWidget.generated.h"

class URhythmLeaderBoard;
enum class EInstrumentType : uint8;
class URhythmSpawnWidgetBase;
class UCanvasPanel;
class URhythmSpawnWidget;

UCLASS(Abstract)
class TROMBONERUMBLE_API URhythmUIRootWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void PrepareNoteContainer(const EInstrumentType& InType);
	void OnGameEnded();
	
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> ShowLeaderboardAnim;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URhythmLeaderBoard> WBP_LeaderBoard;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> NoteCanvas = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<URhythmSpawnWidgetBase> RhythmSpawnWidget = nullptr;
};