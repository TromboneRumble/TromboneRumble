// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RhythmResultWidgetBase.h"
#include "RhythmResultWidgetSquare.generated.h"

class URhythmSpawnWidgetBase;
class UTextBlock;
enum class ENoteResult : uint8;

/**
 * 
 */
UCLASS(Abstract)
class TROMBONERUMBLE_API URhythmResultWidgetSquare : public URhythmResultWidgetBase
{
	GENERATED_BODY()

public:
	void PlayAnimationOnResult(ENoteResult InResult);
protected:
	virtual void NativePreConstruct() override;
	virtual void NativeDestruct() override;

private:

	UFUNCTION()
	void OnDetectedAnimationFinished();

	UPROPERTY(EditDefaultsOnly, Transient, meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> ScoreText = nullptr;

	UPROPERTY(EditDefaultsOnly, Transient, meta = (BindWidgetAnim, AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetAnimation> Detected = nullptr;

	UPROPERTY(Transient)
	TWeakObjectPtr<URhythmSpawnWidgetBase> OwnerSpawnWidget;
};
