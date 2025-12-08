// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Utilities/Defines.h"
#include "RhythmNoteUIControllerComponent.generated.h"


class URhythmNoteWidgetBase;
class URhythmSpawnWidgetBase;

/*
 * RhythmNote랑 RhythmNoteWidget을 연결해주는 Controller역할
 */
UCLASS( ClassGroup=(UI), meta=(BlueprintSpawnableComponent) )
class TROMBONERUMBLE_API URhythmNoteUIControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	URhythmNoteUIControllerComponent();

	void InitSettings(URhythmSpawnWidgetBase* InSpawnWidget, URhythmNoteWidgetBase* InNoteWidget, const FNoteHandle& InHandle);
	void SpawnRhythmResultWidget(ENoteResult InResult);
	
protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void BindChannel();
	void UnbindChannel();
	void UpdateNotePosition(const float InAlpha);

	UPROPERTY(Transient)
	FNoteHandle Handle;
	UPROPERTY(Transient)
	TWeakObjectPtr<URhythmSpawnWidgetBase> RhythmSpawnWidget = nullptr;
	UPROPERTY(Transient)
	TWeakObjectPtr<URhythmNoteWidgetBase> RhythmNoteWidget = nullptr;

	FDelegateHandle ProgressHandle, DespawnHandle;
};
