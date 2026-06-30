// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonRotator.h"
#include "CommonRotatorWidgetBase.generated.h"

UCLASS()
class TROMBONERUMBLE_API UCommonRotatorWidgetBase : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	
	/** Delegate for when the rotator is rotated with a direction. Provides the new index and the direction of rotation. */
	FOnRotatedWithDirection& OnRotatedWithDirection() const { return CR_Rotator->OnRotatedWithDirection; }
	
protected:
	
	UPROPERTY(EditAnywhere, Category = "Options", meta = (AllowPrivateAccess = "true"))
	TArray<FText> TextOptions;
	
	UPROPERTY(EditAnywhere, Category = "Options", meta = (AllowPrivateAccess = "true"))
	int32 DefaultSelectedIndex = 0;

protected:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonRotator> CR_Rotator;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Prev;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CB_Next;
	
private:
	
	void InitButtons();	
	void RefreshRotator();
	
public:
	
	// ~ Begin UUserWidget Interface
	virtual bool Initialize() override;
	virtual void SetIsEnabled(bool bInIsEnabled) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	// ~ End UUserWidget Interface
	
protected:
	
	// ~ Begin UUserWidget Interface
	virtual void NativePreConstruct() override;
	// ~ End UUserWidget Interface

public:
	
	// ~ Begin Getter & Setter
	int32 GetCurrentIndex() const { return CR_Rotator ? CR_Rotator->GetSelectedIndex() : -1; }
	void SetSelectedIndex(int32 NewIndex);
	const TArray<FText>& GetOptions() const { return TextOptions; }
	void SetOptions(const TArray<FText>& InOptions);
	// ~ End Getter & Setter
};
