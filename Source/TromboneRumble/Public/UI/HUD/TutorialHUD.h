#pragma once

#include "CoreMinimal.h"
#include "UI/HUD/BaseHUD.h"
#include "TutorialHUD.generated.h"

UCLASS()
class TROMBONERUMBLE_API ATutorialHUD : public ABaseHUD
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	
};
