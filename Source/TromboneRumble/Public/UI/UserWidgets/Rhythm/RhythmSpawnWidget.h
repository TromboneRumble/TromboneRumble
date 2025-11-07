// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RhythmSpawnWidget.generated.h"

class UCanvasPanel;
class URhythmNoteWidget;

/**
 * 
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API URhythmSpawnWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Note")
	URhythmNoteWidget* SpawnRhythmNoteWidget(int32 LaneIndex);

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> NoteCanvas = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	TSubclassOf<URhythmNoteWidget> NoteWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Note")
	int32 MaxLanes = 3;

protected:
	virtual void NativeConstruct() override;

private:
	void OnViewPortResizedHandler(FViewport* ViewPort, uint32);
	void SetStartPoses();
	float GetLaneY(int32 LaneIndex) const;

	UPROPERTY(Transient)
	float LaneXStartPos = 0.f;

	UPROPERTY(Transient)
	float LaneXEndPos = 0.f;

	UPROPERTY(Transient)
	TArray<float> LaneYPosArray;

	UPROPERTY(Transient)
	bool bInitializedPositions = false;
};
