// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/ToastData.h"
#include "Engine/DeveloperSettings.h"
#include "ToastSettings.generated.h"

class UStringTable;
class UToastContainerWidget;
class UToastItemWidget;

/**
 * Settings of the toast system, shown in Project Settings as Toast Settings.
 * UToastSubsystem reads them once when the game instance starts.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Toast Settings"))
class TROMBONERUMBLE_API UToastSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	/** Widget that holds the toasts on screen. It is created once per map and added to the viewport. */
	UPROPERTY(Config, NoClear, EditAnywhere, Category = "Widgets")
	TSoftClassPtr<UToastContainerWidget> ContainerWidgetClass;

	/** Widget created for each toast and reused through a pool. */
	UPROPERTY(Config, NoClear, EditAnywhere, Category = "Widgets")
	TSoftClassPtr<UToastItemWidget> ItemWidgetClass;

	/** What happens when a toast is requested while another toast is showing. */
	UPROPERTY(Config, EditAnywhere, Category = "Behavior")
	EToastSystemPolicy Policy = EToastSystemPolicy::Queue;

	/** Where each toast appears on screen. */
	UPROPERTY(Config, EditAnywhere, Category = "Behavior")
	EToastPosition Position = EToastPosition::BottomCenter;

	/** How long each toast stays on screen in seconds, not counting its open and close animations. */
	UPROPERTY(Config, EditAnywhere, Category = "Behavior", meta = (ClampMin = "0.1"))
	float DisplayDuration = 2.0f;

	/** String table that holds the text of each notice, keyed by the name of the notice tag. */
	UPROPERTY(Config, EditAnywhere, Category = "Notices")
	TSoftObjectPtr<UStringTable> NoticeStringTable;
};
