#include "UI/HUD/TutorialHUD.h"
#include "DeveloperSettings/TromboneConfig.h"

void ATutorialHUD::BeginPlay()
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
