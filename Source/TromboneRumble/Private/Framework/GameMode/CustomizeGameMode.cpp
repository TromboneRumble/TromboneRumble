#include "Framework/GameMode/CustomizeGameMode.h"
#include "Framework/PlayerController/CustomizePlayerController.h"
#include "UI/HUD/CustomizeHUD.h"

ACustomizeGameMode::ACustomizeGameMode()
{
	PlayerControllerClass = ACustomizePlayerController::StaticClass();
	HUDClass = ACustomizeHUD::StaticClass();
	bUseSeamlessTravel = false;
}
