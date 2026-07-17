// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/Common/BaseMenuWidget.h"

UBaseMenuWidget::UBaseMenuWidget()
{
	// Restore focus to the last focused control when this menu is re-activated (e.g. after a popup closes)
	bAutoRestoreFocus = true;
}

void UBaseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	Init();
}