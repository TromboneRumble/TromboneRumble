// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/UserWidgetPool.h"
#include "RhythmSpawnWidget.generated.h"

enum class ENoteResult : uint8;
enum class EInstrumentType : uint8;
class UCanvasPanel;
class URhythmNoteWidget;
class URhythmResultWidget;

/**
 * 
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API URhythmSpawnWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	URhythmSpawnWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "Note")
	URhythmNoteWidget* GetPooledRhythmNoteWidget(int32 LaneIndex);

	void ReleasePooledRhythmNoteWidget(URhythmNoteWidget* Widget);
	void ReleasePooledRhythmResultWidget(URhythmResultWidget* Widget);
	void SpawnRhythmResultWidget(const FVector2D& SpawnPos, ENoteResult InResult);
	

	UFUNCTION()
	void PlayFadeAnimation(EInstrumentType InType);


public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> NoteCanvas = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	TSubclassOf<URhythmNoteWidget> NoteWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	TSubclassOf<URhythmResultWidget> NoteResultWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Note")
	int32 MaxLanes = 3;


	EInstrumentType InstrumentType;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(Transient)
	FUserWidgetPool WidgetPool;

	void PrewarmWidgetPool();

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

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

	bool bViewportBound = false;
	FDelegateHandle ViewportResizedHandle;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* FadeOutAnim;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* FadeInAnim;

	bool isShown = false;

	
};
