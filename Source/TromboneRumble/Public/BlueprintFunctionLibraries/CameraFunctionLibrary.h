// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CameraFunctionLibrary.generated.h"


class USpringArmComponent;
class AActor;

UENUM(BlueprintType)
enum class ECameraEase : uint8
{
	Linear      UMETA(DisplayName = "Linear"),
	EaseIn      UMETA(DisplayName = "EaseIn (Cubic)"),
	EaseOut     UMETA(DisplayName = "EaseOut (Cubic)"),
	EaseInOut   UMETA(DisplayName = "EaseInOut (Cubic)"),
	Sinusoidal  UMETA(DisplayName = "Sinusoidal"),
	Quintic     UMETA(DisplayName = "Quintic InOut")
};

USTRUCT(BlueprintType)
struct FCameraOffsetLerpState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere) bool   bIsActive = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) float  Elapsed = 0.f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) float  Duration = 0.25f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) FVector Start = FVector::ZeroVector;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) FVector Target = FVector::ZeroVector;
};

/**
 * 
 */
UCLASS()
class TROMBONERUMBLE_API UCameraFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/// <summary>
	/// 
	/// </summary>
	/// <param name="MaxOffset">목표 거리</param>
	/// <param name="InputExponent">민감도 조절 : 1.0기본, 1.5~2.0 미세입력 둔감</param>
	/// <param name="DurationSeconds">목표까지 X초</param>
	/// <param name="bKeepCurrentZ"></param>
	/// <param name="bZeroWhenNoInput">true면 입력 없을때 0으로 복귀</param>
	/// <param name="InOutState"></param>
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static void UpdateTopDownCameraOffsetEase(
		const UObject* WorldContextObject,
		USpringArmComponent* SpringArm,
		AActor* ReferenceActor,
		FVector2D MoveInput,
		UPARAM(ref) FCameraOffsetLerpState& InOutState,
		float MaxOffset = 400.f,
		float InputExponent = 1.f,       // 입력 민감도(>1: 작은 입력 둔감)
		float DurationSeconds = 0.25f,   // 목표까지 걸릴 시간(초)
		ECameraEase EaseType = ECameraEase::EaseOut,
		bool bKeepCurrentZ = true,
		bool bZeroWhenNoInput = true
		
	);
};
