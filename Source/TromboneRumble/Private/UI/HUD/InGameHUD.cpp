#include "UI/HUD/InGameHUD.h"
#include "Blueprint/UserWidget.h"
#include "DeveloperSettings/TromboneConfig.h"
#include "UI/UserWidgets/InGame/SubWidgets/PerformanceWidget.h"

void AInGameHUD::BeginPlay()
{
	Super::BeginPlay();
	
	if (const UTromboneConfig* Config = UTromboneConfig::Get())
	{
		if (Config->PerformanceWidgetClass)
		{
			if (UCommonUserWidget* Widget = CreateWidget<UCommonUserWidget>(GetWorld(), Config->PerformanceWidgetClass))
			{
				Widget->AddToViewport(9999);
				Widget->SetVisibility(ESlateVisibility::HitTestInvisible);
			}
		}
	}
}