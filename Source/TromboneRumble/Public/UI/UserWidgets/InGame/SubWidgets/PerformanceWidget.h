#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PerformanceWidget.generated.h"

class UCommonTextBlock;

UCLASS()
class TROMBONERUMBLE_API UPerformanceWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	
	UPerformanceWidget();
	
protected:
	virtual void NativeOnInitialized() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonTextBlock> Text_FPS;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonTextBlock> Text_Ping;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonTextBlock> Text_MS;
    

private:
    void CalculateAndUpdatePerformances(float InDeltaTime);
	
    float UpdateInterval;
    float DeltaTimeAccumulator;
    int32 FrameCount;
};
