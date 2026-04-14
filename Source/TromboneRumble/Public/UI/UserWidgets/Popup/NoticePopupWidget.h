#pragma once

#include "CoreMinimal.h"
#include "PopupWidgetBase.h"
#include "NoticePopupWidget.generated.h"

class UCommonTextBlock;

UCLASS()
class TROMBONERUMBLE_API UNoticePopupWidget : public UPopupWidgetBase
{
	GENERATED_BODY()
	
public:
	
	/** Default constructor. */
	UNoticePopupWidget();
	
	void OnInit(const FText& InContent) const;
	void OnInit(const FText& InTitle, const FText& InContent) const;
	
protected:
	
	// ~ Begin Widgets
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonTextBlock> Text_Title;
	
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonTextBlock> Text_Content;
	// ~ End Widgets
	
private:
	
	FString DefaultTitle;
	
};
