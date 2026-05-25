#pragma once

#include "CoreMinimal.h"
#include "PopupBase.h"
#include "NoticePopup.generated.h"

class UCommonTextBlock;

UCLASS()
class TROMBONERUMBLE_API UNoticePopup : public UPopupBase
{
	GENERATED_BODY()
	
public:
	
	/** Initializes the popup */
	void Init(const FText& InContent, const FText& InTitle = FText::GetEmpty(), const FText& InCloseButtonText = FText::GetEmpty()) const;
	
protected:
	
	// ~ Begin Widgets
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonTextBlock> Text_Title;
	
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UCommonTextBlock> Text_Content;
	// ~ End Widgets
	
};
