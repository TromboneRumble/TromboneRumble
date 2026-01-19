// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/Common/BaseUIRoot.h"
#include "MatchUIRoot.generated.h"

class UCommonActivatableWidget;
class UCommonActivatableWidgetStack;

enum class EMatchMenuType : uint8
{
	None,
	MatchMenu,
};

UCLASS()
class TROMBONERUMBLE_API UMatchUIRoot : public UBaseUIRoot
{
	GENERATED_BODY()
	
public:
	void PushMenu(EMatchMenuType InType) const;
};