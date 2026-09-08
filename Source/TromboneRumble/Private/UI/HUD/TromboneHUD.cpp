// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/HUD/TromboneHUD.h"
#include "Subsystems/TromboneUISubsystem.h"
#include "TromboneGamePlayTags.h"
#include "UI/UserWidgets/Common/RootUI.h"

void ATromboneHUD::BeginPlay()
{
	Super::BeginPlay();

	UTromboneUISubsystem* UISubsystem = UTromboneUISubsystem::Get(this);
	if (!UISubsystem)
	{
		return;
	}

	// Every level has this HUD, so this is the one place the root gets attached
	UISubsystem->AttachRootUI(GetOwningPlayerController());

	if (URootUI* RootUI = UISubsystem->GetRootUI())
	{
		if (ScreenClass)
		{
			RootUI->AddWidgetToStack(ScreenClass, TromboneGamePlayTags::Trombone_UI_Layer_Base);
		}
		RootUI->SetPerformanceWidgetVisible(bShowPerformanceWidget);
	}
}
