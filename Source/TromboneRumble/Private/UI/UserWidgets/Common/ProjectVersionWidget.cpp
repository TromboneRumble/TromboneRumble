// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/Common/ProjectVersionWidget.h"
#include "CommonTextBlock.h"
#include "GeneralProjectSettings.h"
#include "Kismet/KismetSystemLibrary.h"

void UProjectVersionWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	const FString ProjectVersion = GetDefault<UGeneralProjectSettings>()->ProjectVersion;
	
	if (Text_Version)
	{
		Text_Version->SetText(FText::FromString(ProjectVersion));
	}
}
