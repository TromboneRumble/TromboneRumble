// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RhythmTimeWidget.generated.h"


class UProgressBar;
class UTextBlock;
class UImage;
enum class ERhythmGameState : uint8;


/**
 * 
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API URhythmTimeWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void ResetProgressBar();
protected:
	virtual void NativePreConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> MusicProgressBar;

	// 수동으로 움직여줄 하얀색 네모 상자 (Thumb)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ProgressThumb;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TimeText;

private:
	UFUNCTION()
	void HandleRhythmGameStateChanged(ERhythmGameState RhythmGameState);

	void UpdateTimeUI(float DeltaSeconds);
	void UpdateProgressVisuals(float Percent);

	bool bHasGameStarted = false;
	bool bIsSongPaused = false;

	float CurrentSongTotalLength = 0.f;
	float CurrentTime = 0.f;
	int32 CurrentSongPlayingID = 0;
};
