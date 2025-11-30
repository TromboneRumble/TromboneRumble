// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RhythmSpawnWidgetBase.h"
#include "RhythmSpawnWidgetSquare.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API URhythmSpawnWidgetSquare : public URhythmSpawnWidgetBase
{

	GENERATED_BODY()
public:
	virtual void Init(ARhythmNoteSpawner* InNoteSpawner) override;

	virtual URhythmNoteWidgetBase* SpawnPooledRhythmNoteWidget() override;

	virtual URhythmResultWidgetBase* SpawnPooledRhythmResultWidget(const FVector2D& SpawnPos, ENoteResult InResult) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Note")
	int32 MaxLanes = 3;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void PlayFadeAnimation(EInstrumentType InType);
	void OnViewPortResizedHandler(FViewport* ViewPort, uint32);
	void SetStartPoses();

	UPROPERTY(Transient)
	float LaneXStartPos = 0.f;

	UPROPERTY(Transient)
	float LaneXEndPos = 0.f;

	UPROPERTY(Transient)
	TArray<float> LaneYPosArray;

	UPROPERTY(Transient)
	bool bInitializedPositions = false;

	bool bViewportBound = false;
	FDelegateHandle ViewportResizedHandle;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* FadeOutAnim;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* FadeInAnim;

	bool isShown = false;
	
};
