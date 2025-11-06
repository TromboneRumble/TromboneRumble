// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Utilities/Defines.h"
#include "RhythmNoteUIControllerComponent.generated.h"


class URhythmSpawnWidget;
class URhythmNoteWidget;

UCLASS( ClassGroup=(Game), meta=(BlueprintSpawnableComponent) )
class TROMBONERUMBLE_API URhythmNoteUIControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	URhythmNoteUIControllerComponent();

	void InitSettings(URhythmSpawnWidget* InSpawnWidget, URhythmNoteWidget* InNoteWidget, int32 InLaneIndex);
	void SetStartPoses(const float InLaneStartXPos, const float InLaneEndXPos, const TArray<float>& InLaneYPosArray);


	UPROPERTY()
	TObjectPtr<URhythmSpawnWidget> RhythmSpawnWidget = nullptr;

	UPROPERTY()
	TObjectPtr<URhythmNoteWidget> RhythmNoteWidget = nullptr;

	int32 LaneIndex = -1;
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(Transient)
	float LaneXStartPos = 0.f;

	UPROPERTY(Transient)
	float LaneXEndPos = 0.f;

	UPROPERTY(Transient)
	TArray<float> LaneYPosArray;

private:
	FGameplayMessageListenerHandle LayoutChangedHandle;

	void OnViewportChanged(FGameplayTag Channel, const FViewportChangedMessage& InMsg);
		
};
