// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/HUD/TromboneHUD.h"
#include "Subsystems/TromboneUISubsystem.h"
#include "UI/UserWidgets/Common/RootUI.h"
#include "Utilities/Defines.h"

void ATromboneHUD::BeginPlay()
{
	Super::BeginPlay();

	UTromboneUISubsystem* UISubsystem = UTromboneUISubsystem::Get(this);
	if (!UISubsystem)
	{
		return;
	}

	// Attach here as well so the push below never depends on who ran first
	UISubsystem->AttachRootUI(GetOwningPlayerController());

	if (URootUI* RootUI = UISubsystem->GetRootUI())
	{
		if (ScreenClass)
		{
			RootUI->AddWidgetToStack(ScreenClass, EUIStackType::Base);
		}
		RootUI->SetPerformanceWidgetVisible(bShowPerformanceWidget);
	}
}
