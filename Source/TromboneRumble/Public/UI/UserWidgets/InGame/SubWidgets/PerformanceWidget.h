// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PerformanceWidget.generated.h"

class UCommonTextBlock;

UCLASS()
class TROMBONERUMBLE_API UPerformanceWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
protected:
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonTextBlock> Text_FPS;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonTextBlock> Text_Ping;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonTextBlock> Text_MS;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float UpdateInterval = 0.5f;

private:
    void CalculateAndUpdatePerformances(float InDeltaTime);
	
    float DeltaTimeAccumulator = 0.0f;
    int32 FrameCount = 0;
};
