// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RhythmUIRootWidget.generated.h"

class UCanvasPanel;
class URhythmSpawnWidget;
class URhythmNoteWidget;
/**
 * 
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API URhythmUIRootWidget : public UUserWidget
{
	GENERATED_BODY()


public:
	UFUNCTION(BlueprintCallable, Category = "Note")
	URhythmNoteWidget* SpawnNote(int32 LaneIndex);

	UFUNCTION(BlueprintCallable, Category = "Note")
	void UpdateNoteProgress(URhythmNoteWidget* Note, float Alpha01);

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> NoteCanvas = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URhythmSpawnWidget> RhythmSpawnWidget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	TSubclassOf<URhythmNoteWidget> NoteWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Note")
	int32 MaxLanes = 3;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;


private:
	void OnViewPortResizedHandler(FViewport* ViewPort, uint32);

	void SetStartXPos();
	void SetStartYPos(int32 LaneIndex);
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
