// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UIData.generated.h"

UENUM(BlueprintType)
enum class EToastPosition : uint8
{
	TopLeft, TopCenter, TopRight,
	CenterLeft, Center, CenterRight,
	BottomLeft, BottomCenter, BottomRight,
	MAX,
	Random, // for debug/test
};

UENUM()
enum class EToastTweenType : uint8
{
	AnimIn,
	AnimOut,
};

UENUM(BlueprintType)
enum class EToastSystemPolicy : uint8
{
	/** Wait until the previous toast has finished */
	Queue,
	
	/** Replace the current toast, immediately starting the new one */
	Override,
};

USTRUCT(BlueprintType)
struct FToastRequest
{
	GENERATED_BODY()
	
	/** Default constructor. */
	FToastRequest() = default;

	explicit FToastRequest(const FText& InMessage, const EToastPosition InPosition = EToastPosition::BottomCenter, const float InDisplayDuration = 2.0f)
		: Message(InMessage)
		, Position(InPosition)
		, DisplayDuration(InDisplayDuration)
	{
	}
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FText Message = FText::GetEmpty();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EToastPosition Position = EToastPosition::BottomCenter;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float DisplayDuration = 2.0f;
};