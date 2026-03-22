// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PodiumNameWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class TROMBONERUMBLE_API UPodiumNameWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetPlayerName(const FString& InName);


protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerNameText;
};
