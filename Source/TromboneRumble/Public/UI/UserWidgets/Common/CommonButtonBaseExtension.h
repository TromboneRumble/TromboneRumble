// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "CommonButtonBaseExtension.generated.h"

class UCommonButtonStyleExtension;

/**
 * UCommonButtonStyle은 NormalPressed 상태일 때 UCommonTextStyle 적용과 WwiseEvent 호출을 지원하지 않고,
 * NormalHovered 상태일 때 WwiseEvent 호출을 지원하지 않기에 이 확장이 필요합니다.
 * 
 * UCommonButtonBaseExtension는 텍스트 위젯을 소유하지 않습니다.
 * 텍스트가 필요한 버튼은 UCommonButtonBaseExtensionWithText를 사용하세요.
 * 
 * @see UCommonButtonStyleExtension
 * @see UCommonButtonBaseExtensionWithText
 */
UCLASS()
class TROMBONERUMBLE_API UCommonButtonBaseExtension : public UCommonButtonBase
{
	GENERATED_BODY()
	
protected:
	
	//~ Begin UCommonButtonBase Interface
	virtual void NativeOnHovered() override;
	virtual void NativeOnPressed() override;
	virtual void NativeOnReleased() override;
	//~ End UCommonButtonBase Interface
	
	const UCommonButtonStyleExtension* GetStyleExtensionCDO() const;
	
	TSubclassOf<UCommonTextStyle> GetDesiredTextStyleClass() const;
	
};
