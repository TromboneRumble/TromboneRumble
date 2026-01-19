// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BaseHUD.generated.h"

UENUM(BlueprintType)
enum class EInputModeType : uint8
{
	UIOnly,
	GameOnly,
	GameAndUI
};

UCLASS()
class TROMBONERUMBLE_API ABaseHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	TObjectPtr<UUserWidget> GetRootLayout() const { return RootLayout; }
	
protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<UUserWidget> RootLayoutClass;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> RootLayout;
};
