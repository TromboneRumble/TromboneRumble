// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "CommonButtonStyleExtension.generated.h"

class UAkAudioEvent;

/** 
 * UCommonButtonStyle은 NormalPressed 상태일 때 UCommonTextStyle 적용과 WwiseEvent 호출을 지원하지 않고,
 * NormalHovered 상태일 때 WwiseEvent 호출을 지원하지 않기에 이 확장이 필요합니다.
 * 
 * @see UCommonButtonBaseExtension
 */
UCLASS()
class TROMBONERUMBLE_API UCommonButtonStyleExtension : public UCommonButtonStyle
{
	GENERATED_BODY()
	
public:
	
	/** The text style to use when pressed */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties")
	TSubclassOf<UCommonTextStyle> NormalPressedTextStyle;
	
	/** The sound to play when the button is pressed */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties")
	TObjectPtr<UAkAudioEvent> NormalPressedAudioEvent;
	
	/** The sound to play when the button is hovered */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties")
	TObjectPtr<UAkAudioEvent> NormalHoveredAudioEvent;
	
public:
	
	/** Plays the hovered sound if set. SoundOwner may be null - the sound then plays on a global Wwise object. */
	void PostHoveredSound(AActor* SoundOwner) const;
	
	/** Plays the pressed sound if set. SoundOwner may be null - the sound then plays on a global Wwise object. */
	void PostPressedSound(AActor* SoundOwner) const;
	
};
