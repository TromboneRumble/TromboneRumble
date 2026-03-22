// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BaseHUD.generated.h"

class UBaseUIRoot;

UCLASS()
class TROMBONERUMBLE_API ABaseHUD : public AHUD
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<UBaseUIRoot> RootUIClass;
	
	UPROPERTY()
	TObjectPtr<UBaseUIRoot> RootUI;
	
public:
	// ~ Begin Getter & Setter
	TObjectPtr<UBaseUIRoot> GetRootUI() const { return RootUI; }
	// ~ End Getter & Setter
};
