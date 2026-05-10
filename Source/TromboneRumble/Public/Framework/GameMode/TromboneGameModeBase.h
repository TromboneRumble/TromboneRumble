#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TromboneGameModeBase.generated.h"

UCLASS()
class TROMBONERUMBLE_API ATromboneGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	/** Default constructor. */
	ATromboneGameModeBase();
	
public:

	// ~ Begin AGameModeBase interface
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* ExitedPlayer) override;
	// ~ End AGameModeBase interface
	
};
