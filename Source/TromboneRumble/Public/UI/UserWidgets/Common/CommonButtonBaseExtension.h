// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "CommonButtonBaseExtension.generated.h"

class UCommonButtonStyleExtension;
class UCommonTextBlock;
class UImage;

/**
 * UCommonButtonStyle에는 NormalPressed 텍스트 스타일, Wwise 이벤트를 지원하지 않습니다.
 * 추가로 상태별 버튼 아이콘 불투명도가 필요하고, 이 셋은 UCommonButtonStyleExtension에 있습니다.
 *
 * Text_ActionName으로 바인딩된 텍스트 하나는 여기서 적용하고, 텍스트가 더 있는 버튼은 블루프린트에서 ApplyTextStyle로 구현하면 됩니다.
 *
 * @see UCommonButtonStyleExtension
 */
UCLASS()
class TROMBONERUMBLE_API UCommonButtonBaseExtension : public UCommonButtonBase
{
	GENERATED_BODY()
	
public:
	
	/** The text style for the state the button is in now. Pressed wins when the style has one. */
	UFUNCTION(BlueprintPure, Category = "Common Button|Getters")
	TSubclassOf<UCommonTextStyle> GetDesiredTextStyleClass() const;

	/** Sets Text_ActionName. Does nothing on a button without one. */
	UFUNCTION(BlueprintCallable, Category = "Common Button")
	void SetText(const FText& InText);
	
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
	virtual void NativeOnCurrentTextStyleChanged() override;
	//~ End UCommonButtonBase Interface

	/** Called with the text style for the current state. Implement it on a button whose texts are not Text_ActionName. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Common Button")
	void ApplyTextStyle(TSubclassOf<UCommonTextStyle> TextStyle);
	
protected:
	
	/** Plays the hovered sound if the style has one. */
	void PlayHoveredSound() const;
	
	/** Plays the pressed sound if the style has one. */
	void PlayPressedSound() const;
	
	/** Applies the style's opacity for the state the button is in now. */
	void RefreshImageOpacity() const;
	
	const UCommonButtonStyleExtension* GetStyleExtensionCDO() const;
	
protected:
	
	/** Optional. The one text most buttons have. Its style follows the button state. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Button Config")
	TObjectPtr<UCommonTextBlock> Text_ActionName;

	/** Optional. An icon or other picture that is different per button. The background comes from the style's brushes. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Button Config")
	TObjectPtr<UImage> Image_Button;
	
	/** Fills Image_Button. A texture, or a plain color with no texture and a Box draw type.
	 *  Put the color in the brush tint - ColorAndOpacity belongs to the style's opacity. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Button Config")
	FSlateBrush ButtonBrush;
	
};
