// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "CommonButtonBaseExtension.generated.h"

class UCommonButtonStyleExtension;
class UImage;

/**
 * UCommonButtonStyle은 NormalPressed 상태의 UCommonTextStyle, NormalHovered/NormalPressed 상태의 WwiseEvent,
 * 상태별 이미지 불투명도를 지원하지 않기에 이 확장이 필요합니다.
 * 
 * UCommonButtonBaseExtension는 텍스트 위젯을 소유하지 않습니다.
 * 텍스트 스타일의 선택은 여기서 하고, 적용은 텍스트를 가진 파생 클래스가 NativeOnCurrentTextStyleChanged에서 합니다.
 * 엔진의 UCommonButtonBase(선택)와 UCommonBoundActionButton(적용)이 나누는 방식과 같습니다.
 * 텍스트가 하나인 버튼은 UCommonButtonBaseExtensionWithText를 사용하세요.
 * 
 * @see UCommonButtonStyleExtension
 * @see UCommonButtonBaseExtensionWithText
 */
UCLASS()
class TROMBONERUMBLE_API UCommonButtonBaseExtension : public UCommonButtonBase
{
	GENERATED_BODY()
	
public:
	
	/** The text style for the state the button is in now. Pressed wins when the style has one. */
	UFUNCTION(BlueprintPure, Category = "Common Button|Getters")
	TSubclassOf<UCommonTextStyle> GetDesiredTextStyleClass() const;
	
protected:
	
	//~ Begin UUserWidget Interface
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	//~ End UUserWidget Interface
	
	//~ Begin UCommonButtonBase Interface
	virtual void NativeOnHovered() override;
	virtual void NativeOnUnhovered() override;
	virtual void NativeOnPressed() override;
	virtual void NativeOnReleased() override;
	//~ End UCommonButtonBase Interface
	
protected:
	
	/** Plays the hovered sound if the style has one. */
	void PlayHoveredSound() const;
	
	/** Plays the pressed sound if the style has one. */
	void PlayPressedSound() const;
	
	/** Applies the style's opacity for the state the button is in now. */
	void RefreshImageOpacity() const;
	
	const UCommonButtonStyleExtension* GetStyleExtensionCDO() const;
	
protected:
	
	/** Optional. Each button picks its own picture, the style says how that picture reacts. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Button Config")
	TObjectPtr<UImage> Image_Button;
	
	/** Fills Image_Button. A texture, or a plain color with no texture and a Box draw type.
	 *  Put the color in the brush tint - ColorAndOpacity belongs to the style's opacity. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Button Config")
	FSlateBrush ButtonBrush;
	
};
