// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "MatchUIRoot.generated.h"

class UCommonActivatableWidget;
class UCommonActivatableWidgetStack;

enum class EMatchMenuType : uint8
{
	None,
	MatchMenu,
};

UCLASS()
class TROMBONERUMBLE_API UMatchUIRoot : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativePreConstruct() override;
	virtual void NativeDestruct() override;
	void PushMenu(EMatchMenuType InType) const;
	
protected:
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> MenuStack;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCommonActivatableWidget> MatchMenuWidgetClass;
};