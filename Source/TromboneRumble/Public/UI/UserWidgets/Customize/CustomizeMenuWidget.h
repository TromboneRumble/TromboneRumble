#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgets/Common/BaseMenuWidget.h"
#include "Data/CustomizationSaveData.h"
#include "CustomizeMenuWidget.generated.h"

class UCommonButtonBase;
class UCustomizationComponent;

UCLASS()
class TROMBONERUMBLE_API UCustomizeMenuWidget : public UBaseMenuWidget
{
	GENERATED_BODY()

protected:
	virtual void Init() override;
	virtual void NativeOnActivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_AntennaNext;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_AntennaPrev;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_FaceNext;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_FacePrev;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_CostumeNext;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_CostumePrev;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Randomize;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Apply;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Back;

private:
	UPROPERTY()
	TObjectPtr<UCustomizationComponent> CustomizationComp;

	FCustomizationSaveData OriginalSaveData;

	UFUNCTION() void Handle_AntennaNext();
	UFUNCTION() void Handle_AntennaPrev();
	UFUNCTION() void Handle_FaceNext();
	UFUNCTION() void Handle_FacePrev();
	UFUNCTION() void Handle_CostumeNext();
	UFUNCTION() void Handle_CostumePrev();
	UFUNCTION() void Handle_Randomize();
	UFUNCTION() void Handle_Apply();
	UFUNCTION() void Handle_Back();

	void ShowBackPopup();

	/** CustomizationComp 캐싱 + 디스크 저장 데이터를 dirty 비교 베이스라인으로 캡처 */
	void CaptureBaseline();
};
