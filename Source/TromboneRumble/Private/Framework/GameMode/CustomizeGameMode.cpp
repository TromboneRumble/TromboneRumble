#include "Framework/GameMode/CustomizeGameMode.h"
#include "Framework/PlayerController/CustomizePlayerController.h"

ACustomizeGameMode::ACustomizeGameMode()
{
	PlayerControllerClass = ACustomizePlayerController::StaticClass();
	bUseSeamlessTravel = false;
}
