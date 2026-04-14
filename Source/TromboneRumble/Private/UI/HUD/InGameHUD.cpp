#include "UI/HUD/InGameHUD.h"
#include "Blueprint/UserWidget.h"
#include "UI/UserWidgets/InGame/SubWidgets/PerformanceWidget.h"

void AInGameHUD::BeginPlay()
{
	Super::BeginPlay();
	
	// TODO : 얘는 어떻게 관리할까 단순히 z-order 높게?
	if (PerformanceWidgetClass)
	{
		if (UPerformanceWidget* PerformanceWidget = CreateWidget<UPerformanceWidget>(GetWorld(), PerformanceWidgetClass))
		{
			PerformanceWidget->AddToViewport();
		}
	}
}