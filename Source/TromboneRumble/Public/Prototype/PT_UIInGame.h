// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PT_UIInGame.generated.h"

UCLASS()
class TROMBONERUMBLE_API UPT_UIInGame : public UUserWidget
{
	GENERATED_BODY()

public:
	void ShowInteractionHint(bool bShow) const;

protected:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UHorizontalBox> InteractionHint;
};
