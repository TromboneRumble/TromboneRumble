// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "CommonButtonStyleExtension.generated.h"

class UAkAudioEvent;

/** 
 * UCommonButtonStyle does not support applying UCommonTextStyle to the NormalPressed state & Calling WwiseEvent on button events
 * @see UCommonButtonBaseExtensionWithText
 */
UCLASS()
class TROMBONERUMBLE_API UCommonButtonStyleExtension : public UCommonButtonStyle
{
	GENERATED_BODY()
	
public:
	
	/** The text style to us when pressed */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties")
	TSubclassOf<UCommonTextStyle> NormalPressedTextStyle;
	
	/** The sound to play when the button is pressed */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties")
	TObjectPtr<UAkAudioEvent> NormalPressedAudioEvent;
	
	/** The sound to play when the button is hovered */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties")
	TObjectPtr<UAkAudioEvent> NormalHoveredAudioEvent;
	
};
