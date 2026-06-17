// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Input/CommonBoundActionButton.h"
#include "CommonButtonBaseExtensionWithText.generated.h"

class UCommonButtonStyleExtension;

/**
 * UCommonButtonBase & UCommonButtonStyle does not support applying UCommonTextStyle to the NormalPressed state & Calling WwiseEvent on button events
 * 기존 UCommonButtonBase에 버튼 이벤트 시 와이즈 이벤트 호출과 Pressed 상태에서 텍스트 스타일 변경을 위해 이 확장을 만들었지만,
 * 종종 텍스트를 사용하지 않는 버튼이 있을 수도 있으나, 문제를 간단히 하기 위해 옵션을 제공하여 우회하겠습니다.
 * @see UCommonButtonStyleExtension
 */
UCLASS()
class TROMBONERUMBLE_API UCommonButtonBaseExtensionWithText : public UCommonBoundActionButton
{
	GENERATED_BODY()
	
public:
	
	/** 참이라면, pressed/release 시 UCommonButtonStyleExtension의 텍스트 스타일로 Text_ActionName의 스타일이 바뀔 수 있습니다.
	 * 텍스트를 사용하지 않는 버튼이라면 true로 설정해 둘 필요가 없습니다.	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Options")
	bool bUseText = true;
	
public:
	
	/** Sets the button text. */
	void SetText(const FText& InText) const;
	
public:
	
	// ~ Begin UCommonButtonBase Interface
	virtual void NativeConstruct() override;
	// ~ End UCommonButtonBase Interface
	
protected:
	
	// ~ Begin UCommonButtonBase Interface
	virtual void NativeOnHovered() override;
	virtual void NativeOnPressed() override;
	virtual void NativeOnReleased() override;
	// ~ End UCommonButtonBase Interface
	
private:
	
	UPROPERTY()
	const UCommonButtonStyleExtension* CachedButtonStyleExtension = nullptr;
	
	/** if NormalPressedTextStyle is applied, OnReleased will reset the text style to NormalTextStyle */
	bool bIsNormalPressedTextStyleApplied = false;
	
};
