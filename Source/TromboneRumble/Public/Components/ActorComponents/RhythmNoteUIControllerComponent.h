// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Utilities/Defines.h"
#include "RhythmNoteUIControllerComponent.generated.h"


class URhythmNoteWidget;

/*
 * RhythmNote랑 RhythmNoteWidget을 연결해주는 Controller역할
 */
UCLASS( ClassGroup=(UI), meta=(BlueprintSpawnableComponent) )
class TROMBONERUMBLE_API URhythmNoteUIControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	URhythmNoteUIControllerComponent();

	void InitSettings(URhythmNoteWidget* InNoteWidget, const FNoteHandle& InHandle, const int32 InLineIdx);
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void BindChannel();
	void UnbindChannel();
	void OnViewportChanged(FGameplayTag Channel, const FViewportChangedMessage& InMsg);
	void SetStartPoses(const float InLaneStartXPos, const float InLaneEndXPos, const TArray<float>& InLaneYPosArray);
	void UpdateNotePosition(const float InAlphaOnSpline);

	UPROPERTY(Transient)
	FNoteHandle Handle;
	UPROPERTY(Transient)
	TWeakObjectPtr<URhythmNoteWidget> RhythmNoteWidget = nullptr;

	FDelegateHandle ProgressHandle, DespawnHandle;

	float LaneXStartPos = 0.f;
	float LaneXEndPos = 0.f;
	TArray<float> LaneYPosArray;
	int32 LaneIndex = 0;

	FGameplayMessageListenerHandle LayoutChangedHandle;
	
		
};
