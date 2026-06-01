#include "UI/HUD/BaseHUD.h"
#include "Blueprint/UserWidget.h"
#include "DeveloperSettings/TromboneConfig.h"
#include "UI/UserWidgets/Common/BaseUIRoot.h"

void ABaseHUD::BeginPlay()
{
	Super::BeginPlay();
	
	if (RootUIClass)
	{
		RootUI = CreateWidget<UBaseUIRoot>(GetWorld(), RootUIClass);
		if (RootUI)
		{
			RootUI->AddToViewport(1000);
		}
	}
	
	if (const UTromboneConfig* Config = UTromboneConfig::Get())
	{
		if (Config->ProjectVersionWidgetClass)
		{
			if (UCommonUserWidget* Widget = CreateWidget<UCommonUserWidget>(GetWorld(), Config->ProjectVersionWidgetClass))
			{
				Widget->AddToViewport(9999);
				Widget->SetVisibility(ESlateVisibility::HitTestInvisible);
			}
		}
	}
}
