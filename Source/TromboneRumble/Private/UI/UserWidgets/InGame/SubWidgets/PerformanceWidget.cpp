#include "UI/UserWidgets/InGame/SubWidgets/PerformanceWidget.h"
#include "CommonTextBlock.h"
#include "DeveloperSettings/TromboneConfig.h"
#include "GameFramework/PlayerState.h"

UPerformanceWidget::UPerformanceWidget()
	: UpdateInterval(0.5f),
	  DeltaTimeAccumulator(0.0f),
	  FrameCount(0)
{
}

void UPerformanceWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	UpdateInterval = UTromboneConfig::Get()->PerformanceWidgetUpdateInterval;
}

void UPerformanceWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	CalculateAndUpdatePerformances(InDeltaTime);
}

void UPerformanceWidget::CalculateAndUpdatePerformances(const float InDeltaTime)
{
	DeltaTimeAccumulator += InDeltaTime;
	FrameCount++;

	if (DeltaTimeAccumulator >= UpdateInterval)
	{
		const float AvgFPS = FrameCount / DeltaTimeAccumulator;
		const float AvgMS = (DeltaTimeAccumulator / FrameCount) * 1000.0f;

		if (Text_FPS)
		{
			Text_FPS->SetText(FText::Format(FText::FromString(TEXT("{0} FPS")), FText::AsNumber(FMath::RoundToInt(AvgFPS))));
		}
		if (Text_MS)
		{
			Text_MS->SetText(FText::Format(FText::FromString(TEXT("{0} ms")), FText::AsNumber(FMath::RoundToFloat(AvgMS * 100.0f) / 100.0f)));
		}

		DeltaTimeAccumulator = 0.0f;
		FrameCount = 0;
	}

	if (const APlayerController* PC = GetOwningPlayer())
	{
		if (const APlayerState* PS = PC->PlayerState)
		{
			const float CurrentPing = PS->GetPingInMilliseconds(); 
            
			if (Text_Ping)
			{
				Text_Ping->SetText(FText::Format(FText::FromString(TEXT("{0} Ping")), FText::AsNumber(FMath::RoundToInt(CurrentPing))));
			}
		}
	}
}
