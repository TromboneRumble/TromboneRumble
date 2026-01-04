// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OSI_WidgetBase.generated.h"

class UImage;
/**
 * 
 */
UCLASS(Abstract, meta = (DisableNativeTick))
class TROMBONERUMBLE_API UOSI_WidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TObjectPtr<USceneComponent> TargetComponent;
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void OSITimer();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UImage> IndicatorIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> NonPointingIndicatorTex = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> PointingIndicatorTex = nullptr;


private:
	void UpdateViewportSize();
	bool IsWorldLocationWithinScreenClamp(const FVector& InWorldPosition);
	void UpdateWidgetLocation(bool IsOnScreen);
	void UpdateSpriteAngle(bool IsOnScreen);


	FTimerHandle TimerHandle_Update;

	FVector2D SavedViewportSize;
	FVector2D ClampMin;
	FVector2D ClampMax;


	FVector ObjectLocation;
	FVector ObjectDirection;
	FVector2D WidgetScreenLocation;

	//Viewport Middle Point to World Space
	FVector MiddlePoint;
	//Viewport MiddlePoint + Object에 향한 방향벡터 * Accuracy
	FVector MidPointTowardObject;
	float Accuracy = 2.0f;

	FVector2D ScreenMiddle2D;
	FVector2D RunAndRise;
	float LineLength;
};
