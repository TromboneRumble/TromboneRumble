// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/UserWidgetPool.h"
#include "RhythmSpawnWidgetBase.generated.h"


class ARhythmNoteSpawner;
enum class ENoteResult : uint8;
enum class EInstrumentType : uint8;
class UCanvasPanel;
class UPanelWidget;
class URhythmNoteWidgetBase;
class URhythmResultWidgetBase;

/**
 * 
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API URhythmSpawnWidgetBase : public UUserWidget
{
	GENERATED_BODY()
public:
	URhythmSpawnWidgetBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void PrepareNoteContainer(const EInstrumentType& InType);

	virtual URhythmNoteWidgetBase* SpawnPooledRhythmNoteWidget(const EInstrumentType& InType);
	virtual URhythmResultWidgetBase* SpawnPooledRhythmResultWidget(const FVector2D& SpawnPos, ENoteResult InNoteResult);

	virtual void ReleasePooledRhythmNoteWidget(URhythmNoteWidgetBase* Widget);
	virtual void ReleasePooledRhythmResultWidget(URhythmResultWidgetBase* Widget);

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> NoteCanvas = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	TSubclassOf<URhythmNoteWidgetBase> NoteWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	TSubclassOf<URhythmResultWidgetBase> NoteResultWidgetClass;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	UFUNCTION()
	void OnInstrumentChangedHandler(EInstrumentType PrevType, EInstrumentType NewType);

	UPROPERTY(Transient)
	FUserWidgetPool WidgetPool;

	// 각 악기별 NoteWidget을 담을 CanvasPanel
	UPROPERTY(Transient)
	TMap<EInstrumentType, TObjectPtr<UPanelWidget>> InstrumentContainers;

	bool isShown = false;

private:
	void PrepareRhythmResultWidgets();
};
