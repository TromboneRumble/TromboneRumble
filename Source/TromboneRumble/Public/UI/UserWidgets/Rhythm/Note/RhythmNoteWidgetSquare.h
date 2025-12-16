// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RhythmNoteWidgetBase.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "RhythmNoteWidgetSquare.generated.h"

class UCanvasPanelSlot;
struct FViewportChangedMessage;
/**
 * 
 */
UCLASS(Abstract, meta = (DisableNativeTick))
class TROMBONERUMBLE_API URhythmNoteWidgetSquare : public URhythmNoteWidgetBase
{
	GENERATED_BODY()

public:
	virtual void Init(const EInstrumentType& InType) override;
	virtual void InitWithCueMessage(const FString& InUserCueName) override;
	virtual void UpdateNotePosition(const float InAlpha) override;

	FORCEINLINE void SetStartPoses(float InLaneXStartPos, float InLaneXEndPos, const TArray<float>& InLaneYPosArray)
	{
		LaneXStartPos = InLaneXStartPos;
		LaneXEndPos = InLaneXEndPos;
		LaneYPosArray = InLaneYPosArray;
	}

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

private:
	void OnViewportChanged(FGameplayTag Channel, const FViewportChangedMessage& InMsg);

	FGameplayMessageListenerHandle LayoutChangedHandle;

	UPROPERTY(Transient)
	float LaneXStartPos = 0.f;

	UPROPERTY(Transient)
	float LaneXEndPos = 0.f;

	int32 LaneIndex;

	UPROPERTY(Transient)
	TArray<float> LaneYPosArray;
};
