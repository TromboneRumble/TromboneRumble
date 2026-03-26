// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ChoirDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FChoirFaceFrame
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 FaceIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 0.1f;
};

USTRUCT(BlueprintType)
struct FChoirAnimationSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FChoirFaceFrame> Frames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLoop = true;
};

UCLASS()
class TROMBONERUMBLE_API UChoirDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, Category = "Animations")
    FChoirAnimationSequence IdleAnimation;

    UPROPERTY(EditAnywhere, Category = "Animations")
    FChoirAnimationSequence SingAnimation;

    UPROPERTY(EditAnywhere, Category = "Config|Random", meta = (ClampMin = "0.1"))
    float MinPlayRate = 0.8f;

    UPROPERTY(EditAnywhere, Category = "Config|Random", meta = (ClampMin = "0.1"))
    float MaxPlayRate = 1.2f;

    // 시작 시 최대 랜덤 지연 시간
    UPROPERTY(EditAnywhere, Category = "Config|Random", meta = (ClampMin = "0.0"))
    float MaxStartOffset = 1.0f;
};
