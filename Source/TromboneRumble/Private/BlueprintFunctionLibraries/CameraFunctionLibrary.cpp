// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintFunctionLibraries/CameraFunctionLibrary.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Actor.h"


static float ApplyEase(float T, ECameraEase Type)
{
    T = FMath::Clamp(T, 0.f, 1.f);
    switch (Type)
    {
    case ECameraEase::Linear:     return T;
    case ECameraEase::EaseIn:     return T * T * T;                                // cubic in
    case ECameraEase::EaseOut: { float U = 1.f - T; return 1.f - U * U * U; }       // cubic out
    case ECameraEase::EaseInOut:  return FMath::InterpEaseInOut(0.f, 1.f, T, 3.f); // cubic in/out
    case ECameraEase::Sinusoidal: return 0.5f - 0.5f * FMath::Cos(PI * T);         // smooth sine
    case ECameraEase::Quintic:    return FMath::InterpEaseInOut(0.f, 1.f, T, 5.f);
    default:                      return T;
    }
}

static FVector MakeDesiredWorldOffsetFromCameraYaw(
    const USpringArmComponent* SpringArm, const FVector2D MoveInput, float MaxOffset, float Strength)
{
    const FRotator CamYaw(0.f, SpringArm->GetComponentRotation().Yaw, 0.f);
    const FVector  CamFwd = CamYaw.Vector();
    const FVector  CamRight = FRotationMatrix(CamYaw).GetUnitAxis(EAxis::Y);
    FVector Desired = FVector::ZeroVector;

    if (Strength > KINDA_SMALL_NUMBER)
    {
        const FVector DirWorld = (CamRight * MoveInput.X + CamFwd * MoveInput.Y).GetSafeNormal();
        Desired = DirWorld * (MaxOffset * Strength);
        Desired.Z = 0.f;
    }
    return Desired;
}

void UCameraFunctionLibrary::UpdateTopDownCameraOffsetEase(
    const UObject* WorldContextObject,
    USpringArmComponent* SpringArm,
    AActor* ReferenceActor,
    FVector2D MoveInput,
    FCameraOffsetLerpState& InOutState,
    float MaxOffset,
    float InputExponent,
    float DurationSeconds,
    ECameraEase EaseType,
    bool bKeepCurrentZ,
    bool bZeroWhenNoInput
    
)
{
    if (!WorldContextObject || !SpringArm || !ReferenceActor) return;
    const UWorld* World = WorldContextObject->GetWorld();
    if (!World) return;

    const float DeltaTime = World->GetDeltaSeconds();

    // 입력 정규화
    MoveInput.X = FMath::Clamp(MoveInput.X, -1.f, 1.f);
    MoveInput.Y = FMath::Clamp(MoveInput.Y, -1.f, 1.f);

    // 입력 세기 + 민감도 곡선
    float InputLen = MoveInput.Length();         // 0~√2
    InputLen = FMath::Clamp(InputLen, 0.f, 1.f); // 0~1
    const float Strength = (InputExponent > 0.f) ? FMath::Pow(InputLen, InputExponent) : InputLen;

    // 목표(Target) 계산 (월드 → 로컬 변환)
    FVector DesiredWorld = FVector::ZeroVector;
    if (Strength > KINDA_SMALL_NUMBER)
    {
        DesiredWorld = MakeDesiredWorldOffsetFromCameraYaw(SpringArm, MoveInput, MaxOffset, Strength);
    }
    else if (!bZeroWhenNoInput)
    {
        // 입력 없을 때 유지하고 싶다면: 현재 Target 유지
        DesiredWorld = SpringArm->GetComponentTransform().TransformVectorNoScale(InOutState.Target);
    }
    // 기본: 입력 없을 때 0으로 복귀

    FVector DesiredLocal = SpringArm->GetComponentTransform().InverseTransformVectorNoScale(DesiredWorld);

    // Z 유지 옵션
    if (bKeepCurrentZ)
    {
        DesiredLocal.Z = SpringArm->TargetOffset.Z;
    }

    // 새 목표가 생기면 이징 상태(시작값/시간) 초기화
    const float ToleranceSq = 0.5f; // 필요시 조절
    const bool bNewTarget = !InOutState.bIsActive
        || (DesiredLocal - InOutState.Target).SizeSquared() > ToleranceSq;

    if (bNewTarget)
    {
        InOutState.Start = SpringArm->TargetOffset;
        InOutState.Target = DesiredLocal;
        InOutState.Elapsed = 0.f;
        InOutState.Duration = FMath::Max(0.001f, DurationSeconds);
        InOutState.bIsActive = true;
    }

    // 시간 기반 이징 보간 (정확히 Duration초 사용)
    float Alpha = (InOutState.Duration <= KINDA_SMALL_NUMBER) ? 1.f
        : FMath::Clamp(InOutState.Elapsed / InOutState.Duration, 0.f, 1.f);
    const float Eased = ApplyEase(Alpha, EaseType);

    const FVector NewLocal = FMath::Lerp(InOutState.Start, InOutState.Target, Eased);
    SpringArm->TargetOffset = NewLocal;

    // 시간 진행
    if (InOutState.bIsActive)
    {
        InOutState.Elapsed += DeltaTime;
        if (InOutState.Elapsed >= InOutState.Duration)
        {
            // 정확히 목표에 스냅 & 종료
            SpringArm->TargetOffset = InOutState.Target;
            InOutState.bIsActive = false;
        }
    }
}
