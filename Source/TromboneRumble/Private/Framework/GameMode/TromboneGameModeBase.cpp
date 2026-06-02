#include "Framework/GameMode/TromboneGameModeBase.h"
#include "Framework/DefaultPlayerState.h"
#include "Subsystems/AppearanceSubsystem.h"

ATromboneGameModeBase::ATromboneGameModeBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bUseSeamlessTravel = true;
}

bool ATromboneGameModeBase::AllowCheats(APlayerController* PC)
{
#if UE_BUILD_SHIPPING
	return Super::AllowCheats(PC);
#else
	return true;
#endif
}

void ATromboneGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	if (HasAuthority())
	{
		if (ADefaultPlayerState* PS = NewPlayer->GetPlayerState<ADefaultPlayerState>())
		{
			FLinearColor SkinColor = PS->GetSkinColor();
			if (SkinColor == FLinearColor::Black)
			{
				if (auto* AppearanceSub = GetGameInstance()->GetSubsystem<UAppearanceSubsystem>())
				{
					SkinColor = AppearanceSub->AssignUniqueColor();
				}
			}
			
			PS->SetSkinColor(SkinColor);
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
