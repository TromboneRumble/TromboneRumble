// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Subsystems/TromboneUISubsystem.h"
#include "Blueprint/UserWidget.h"
#include "DeveloperSettings/TromboneConfig.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "UI/UserWidgets/Common/RootUI.h"
#include "Utilities/DebugHelper.h"

namespace
{
	// Same order as the old HUD path so popups still sit above world widgets
	constexpr int32 RootUIZOrder = 1000;
}

UTromboneUISubsystem* UTromboneUISubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (const UGameInstance* GI = World->GetGameInstance())
		{
			return GI->GetSubsystem<UTromboneUISubsystem>();
		}
	}

	return nullptr;
}

bool UTromboneUISubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// A dedicated server has no screen to draw on
	return !CastChecked<UGameInstance>(Outer)->IsDedicatedServerInstance();
}

void UTromboneUISubsystem::Deinitialize()
{
	if (RootUI)
	{
		RootUI->RemoveFromParent();
		RootUI = nullptr;
	}

	Super::Deinitialize();
}

void UTromboneUISubsystem::AttachRootUI(APlayerController* PlayerController)
{
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}

	if (!RootUI)
	{
		const UTromboneConfig* Config = UTromboneConfig::Get();
		const TSubclassOf<URootUI> RootClass = Config ? Config->RootUIClass.LoadSynchronous() : nullptr;
		if (!RootClass)
		{
			LOG_WITH_CURRENT_CONTEXT(Error, TEXT("RootUIClass is not set in Trombone Config."));
			return;
		}

		// The player controller overload makes the game instance the outer, so the widget outlives the level
		RootUI = CreateWidget<URootUI>(PlayerController, RootClass);
		if (!RootUI)
		{
			LOG_WITH_CURRENT_CONTEXT(Error, TEXT("Failed to create the root layout."));
			return;
		}
	}

	// Already on screen for this controller. Re-adding would rebuild Slate and empty every stack
	if (RootUI->IsInViewport() && RootUI->GetOwningPlayer() == PlayerController)
	{
		return;
	}

	// Level travel tears the old Slate tree down. Point the widget at the new controller and add it back
	RootUI->RemoveFromParent();
	RootUI->SetPlayerContext(FLocalPlayerContext(PlayerController));
	RootUI->AddToPlayerScreen(RootUIZOrder);
}
