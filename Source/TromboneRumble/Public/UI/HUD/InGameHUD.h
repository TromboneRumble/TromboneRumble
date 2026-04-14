#pragma once

#include "CoreMinimal.h"
#include "BaseHUD.h"
#include "InGameHUD.generated.h"

class UPerformanceWidget;
class UInGameWidget;

UCLASS()
class TROMBONERUMBLE_API AInGameHUD : public ABaseHUD
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPerformanceWidget> PerformanceWidgetClass;
};
