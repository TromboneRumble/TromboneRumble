#include "Framework/GameMode/TromboneGameModeBase.h"
#include "Framework/DefaultPlayerState.h"
#include "Subsystems/AppearanceSubsystem.h"

ATromboneGameModeBase::ATromboneGameModeBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bUseSeamlessTravel = true;
}

void ATromboneGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	if (HasAuthority())
	{
		if (ADefaultPlayerState* PS = NewPlayer->GetPlayerState<ADefaultPlayerState>())
		{
			if (PS->GetSkinColor() == FLinearColor::Black)
			{
				if (auto* AppearanceSub = GetGameInstance()->GetSubsystem<UAppearanceSubsystem>())
				{
					const FLinearColor UniqueColor = AppearanceSub->AssignUniqueColor();
					PS->SetSkinColor(UniqueColor);
				}
			}
		}
	}
}

void ATromboneGameModeBase::Logout(AController* ExitedPlayer)
{
	Super::Logout(ExitedPlayer);
	
	if (HasAuthority())
	{
		if (const ADefaultPlayerState* PS = ExitedPlayer->GetPlayerState<ADefaultPlayerState>())
		{
			if (auto* AppearanceSub = GetGameInstance()->GetSubsystem<UAppearanceSubsystem>())
			{
				AppearanceSub->ReleaseColor(PS->GetSkinColor());
			}
		}
	}
}
