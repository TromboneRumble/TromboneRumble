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

USTRUCT(BlueprintType)
struct FCameraZoomLerpState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere) float    CurrentArmLength = 800.f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) FRotator CurrentRotation  = FRotator(-30.f, 0.f, 0.f);
	UPROPERTY(BlueprintReadWrite, EditAnywhere) bool     bInitialized     = false;
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

	/// <summary>
	/// 마우스 휠 줌 전용: TargetArmLength와 CameraBoom의 Rotation을 목표값으로 부드럽게 보간하며,
	/// 줌아웃 시 벽에 막히면 최대 가능 거리까지만 뻗도록 커스텀 Sphere Sweep으로 클램프한다.
	/// </summary>
	/// <param name="DesiredArmLength">목표 Arm Length</param>
	/// <param name="DesiredRotation">목표 CameraBoom Rotation (Absolute)</param>
	/// <param name="InOutState">줌 보간 상태(이상값 보존용)</param>
	/// <param name="InterpSpeed">FInterpTo/RInterpTo 속도</param>
	/// <param name="CollisionMargin">벽 여유 거리(cm)</param>
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static void UpdateTopDownCameraZoomEase(
		const UObject* WorldContextObject,
		USpringArmComponent* SpringArm,
		AActor* ReferenceActor,
		float DesiredArmLength,
		FRotator DesiredRotation,
		UPARAM(ref) FCameraZoomLerpState& InOutState,
		float InterpSpeed = 8.f,
		float CollisionMargin = 10.f
	);
};
