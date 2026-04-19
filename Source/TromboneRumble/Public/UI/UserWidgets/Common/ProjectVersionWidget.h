// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "ProjectVersionWidget.generated.h"

class UCommonTextBlock;

UCLASS()
class TROMBONERUMBLE_API UProjectVersionWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
protected:
	
	virtual void NativePreConstruct() override;

	
public:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_Version;
	
};
