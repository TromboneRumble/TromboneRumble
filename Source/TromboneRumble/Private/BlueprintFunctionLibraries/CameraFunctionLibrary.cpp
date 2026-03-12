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

    FVector DesiredWorld_Unclamped = FVector::ZeroVector;
    if (Strength > KINDA_SMALL_NUMBER)
    {
        DesiredWorld_Unclamped = MakeDesiredWorldOffsetFromCameraYaw(SpringArm, MoveInput, MaxOffset, Strength);
    }
    else if (!bZeroWhenNoInput)
    {
        DesiredWorld_Unclamped = SpringArm->GetComponentTransform().TransformVectorNoScale(InOutState.Target);
    }

    FVector DesiredLocal_Unclamped = SpringArm->GetComponentTransform().InverseTransformVectorNoScale(DesiredWorld_Unclamped);

    if (bKeepCurrentZ)
    {
        DesiredLocal_Unclamped.Z = SpringArm->TargetOffset.Z;
    }


    // 이징 상태 갱신 (벽 충돌과 무관하게 입력에만 반응)
    const float ToleranceSq = 0.5f;
    const bool bNewTarget = !InOutState.bIsActive || (DesiredLocal_Unclamped - InOutState.Target).SizeSquared() > ToleranceSq;

    if (bNewTarget)
    {
        InOutState.Start = SpringArm->TargetOffset;
        InOutState.Target = DesiredLocal_Unclamped;
        InOutState.Elapsed = 0.f;
        InOutState.Duration = FMath::Max(0.001f, DurationSeconds);
        InOutState.bIsActive = true;
    }

    // 시간 기반 이징 보간 (충돌 없는 이상적인 궤적)
    if (InOutState.bIsActive)
    {
        InOutState.Elapsed += DeltaTime;
        if (InOutState.Elapsed >= InOutState.Duration)
        {
            InOutState.bIsActive = false;
        }
    }

    float Alpha = (InOutState.Duration <= KINDA_SMALL_NUMBER) ? 1.f : FMath::Clamp(InOutState.Elapsed / InOutState.Duration, 0.f, 1.f);
    const float Eased = ApplyEase(Alpha, EaseType);

    // 이번 프레임에서 카메라가 가고 싶어 하는 이상적인 로컬 위치
    FVector EasedLocal_Unclamped = FMath::Lerp(InOutState.Start, InOutState.Target, Eased);

    // 벽 충돌 검사 (이상적인 위치를 향해 레이캐스트)
    FVector EasedWorld_Unclamped = SpringArm->GetComponentTransform().TransformVectorNoScale(EasedLocal_Unclamped);
    FVector FinalWorld = EasedWorld_Unclamped; // 기본값은 충돌이 없을 때의 값

    if (!EasedWorld_Unclamped.IsNearlyZero())
    {
        FVector TraceStart = ReferenceActor->GetActorLocation();
        FVector TraceEnd = TraceStart + EasedWorld_Unclamped;

        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(ReferenceActor);

        bool bHit = World->SweepSingleByChannel(
            HitResult,
            TraceStart,
            TraceEnd,
            FQuat::Identity,
            SpringArm->ProbeChannel,
            FCollisionShape::MakeSphere(SpringArm->ProbeSize),
            QueryParams
        );

        if (bHit)
        {
            if (HitResult.bStartPenetrating)
            {
                // 이미 벽에 파고든 상태에서 시작했다면, 벽의 표면(Normal)을 따라 미끄러지도록 처리
                FinalWorld = FVector::VectorPlaneProject(EasedWorld_Unclamped, HitResult.Normal);
            }
            else
            {
                // 벽에 부딪힌 경우: 충돌 지점까지는 이동하고, 남은 이동량은 벽면을 따라 미끄러지게(Slide) 만듭니다.
                FVector SafeMove = EasedWorld_Unclamped * FMath::Max(0.f, HitResult.Time - 0.05f); // 안전 마진
                FVector Remainder = EasedWorld_Unclamped * (1.f - HitResult.Time);
                FVector SlidedRemainder = FVector::VectorPlaneProject(Remainder, HitResult.ImpactNormal);

                FinalWorld = SafeMove + SlidedRemainder;
            }

            // 미끄러지는 벡터가 원래 가려던 길이보다 길어지지 않게 제한
            FinalWorld = FinalWorld.GetClampedToMaxSize(EasedWorld_Unclamped.Size());
        }
    }

    // 최종 계산된 월드 좌표를 다시 로컬 좌표로 변환
    FVector FinalLocal = SpringArm->GetComponentTransform().InverseTransformVectorNoScale(FinalWorld);

    // Z축 유지 보정 (충돌로 인해 위아래 오프셋이 흔들리는 것 방지)
    if (bKeepCurrentZ)
    {
        FinalLocal.Z = SpringArm->TargetOffset.Z;
    }

    // 5. 최종 위치 적용 (VInterpTo를 사용해 1프레임 튀는 현상 흡수)
    // 15.f는 보간 속도입니다. 수치가 높을수록 빠릿하게 따라가고, 낮을수록 부드럽습니다.
    SpringArm->TargetOffset = FMath::VInterpTo(SpringArm->TargetOffset, FinalLocal, DeltaTime, 100.f);
}
