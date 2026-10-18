// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintFunctionLibraries/CameraFunctionLibrary.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"


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

// 피벗이 벽에서 멈출 때 남기는 여유 거리(cm)
static constexpr float PivotWallMargin = 2.f;

// 구가 시작부터 벽에 겹쳐 있으면 엔진은 거리 0을 준다(CollisionConversions.cpp:531-533).
// 겹친 깊이만큼 구를 줄여 다시 쏜다. 끝내 못 빠져나오면 false.
static bool SweepFromClearStart(const UWorld* World, ECollisionChannel Channel, const FCollisionQueryParams& Params,
    const FVector& Start, const FVector& End, float& InOutRadius, FHitResult& OutHit)
{
    for (int32 Attempt = 0; Attempt < 4; ++Attempt)
    {
        const bool bHit = World->SweepSingleByChannel(OutHit, Start, End, FQuat::Identity, Channel, FCollisionShape::MakeSphere(InOutRadius), Params);
        if (!bHit || !OutHit.bStartPenetrating)
        {
            OutHit.bBlockingHit = bHit;
            return true;
        }

        if (OutHit.PenetrationDepth <= KINDA_SMALL_NUMBER)
        {
            return false;
        }
        InOutRadius -= OutHit.PenetrationDepth + 1.f;
        if (InOutRadius < 1.f)
        {
            return false;
        }
    }
    return false;
}

// 피벗을 Move(수평)만큼 옮기되, 벽에 막히면 벽면을 따라 미끄러진다. 실제 이동량을 돌려준다.
static FVector MovePivotWithSlide(const UWorld* World, const USpringArmComponent* SpringArm, const AActor* ReferenceActor,
    const FVector& Start, const FVector& Move)
{
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(ReferenceActor);

    // 구가 캡슐보다 크면 캐릭터가 닿은 벽에 처음부터 겹친다. BP의 ProbeSize가 커도 캡슐 안쪽으로 제한한다
    float Radius = SpringArm->ProbeSize;
    if (const ACharacter* Character = Cast<ACharacter>(ReferenceActor))
    {
        Radius = FMath::Min(Radius, Character->GetCapsuleComponent()->GetScaledCapsuleRadius() - PivotWallMargin);
    }
    FHitResult Hit;
    if (!SweepFromClearStart(World, SpringArm->ProbeChannel, QueryParams, Start, Start + Move, Radius, Hit))
    {
        return FVector::ZeroVector;
    }
    if (!Hit.bBlockingHit)
    {
        return Move;
    }

    // 벽 앞까지 간 뒤, 남은 이동에서 벽 쪽 성분만 뺀다
    const FVector Safe = Move.GetSafeNormal() * FMath::Max(0.f, Hit.Distance - PivotWallMargin);
    const FVector WallNormal = FVector(Hit.Normal.X, Hit.Normal.Y, 0.f).GetSafeNormal();
    FVector Slide = FVector::VectorPlaneProject(Move * (1.f - Hit.Time), WallNormal);

    // 미끄러지는 길에 다른 벽(코너)이 있으면 거기서 멈춘다. 방금 닿은 벽은 무시
    if (!Slide.IsNearlyZero())
    {
        QueryParams.bFindInitialOverlaps = false;
        const FVector SlideStart = Start + Safe;
        FHitResult SlideHit;
        if (World->SweepSingleByChannel(SlideHit, SlideStart, SlideStart + Slide, FQuat::Identity,
            SpringArm->ProbeChannel, FCollisionShape::MakeSphere(Radius), QueryParams))
        {
            Slide = Slide.GetSafeNormal() * FMath::Max(0.f, SlideHit.Distance - PivotWallMargin);
        }
    }

    return Safe + Slide;
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

    // 엔진은 TargetOffset을 회전 없이 월드 좌표로 더한다(SpringArmComponent.cpp:130). 그래서 전부 월드 좌표로 계산한다.
    // 예전엔 로컬로 바꿨다가 되돌려서 피치만큼 기울어진 높이 성분이 앞뒤 이동으로 바뀌어 피벗이 벽 안으로 들어갔다.
    FVector Desired = FVector::ZeroVector;
    if (Strength > KINDA_SMALL_NUMBER)
    {
        Desired = MakeDesiredWorldOffsetFromCameraYaw(SpringArm, MoveInput, MaxOffset, Strength);
    }
    else if (!bZeroWhenNoInput)
    {
        Desired = InOutState.Target;
    }

    if (bKeepCurrentZ)
    {
        Desired.Z = SpringArm->TargetOffset.Z;
    }


    // 이징 상태 갱신 (벽 충돌과 무관하게 입력에만 반응)
    const float ToleranceSq = 0.5f;
    const bool bNewTarget = !InOutState.bIsActive || (Desired - InOutState.Target).SizeSquared() > ToleranceSq;

    if (bNewTarget)
    {
        InOutState.Start = SpringArm->TargetOffset;
        InOutState.Target = Desired;
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

    // 이번 프레임에서 카메라가 가고 싶어 하는 이상적인 위치 (충돌 무시)
    const FVector EasedOffset = FMath::Lerp(InOutState.Start, InOutState.Target, Eased);

    // 벽 검사는 수평으로만 한다
    const FVector Move(EasedOffset.X, EasedOffset.Y, 0.f);
    FVector Final = Move;
    if (!Move.IsNearlyZero())
    {
        const FVector PivotBase = SpringArm->GetComponentLocation() + FVector(0.f, 0.f, EasedOffset.Z);
        Final = MovePivotWithSlide(World, SpringArm, ReferenceActor, PivotBase, Move);
    }

    // Z축 유지 보정 (충돌로 인해 위아래 오프셋이 흔들리는 것 방지)
    Final.Z = bKeepCurrentZ ? SpringArm->TargetOffset.Z : EasedOffset.Z;

    // 5. 최종 위치 적용 (VInterpTo를 사용해 1프레임 튀는 현상 흡수)
    // 15.f는 보간 속도입니다. 수치가 높을수록 빠릿하게 따라가고, 낮을수록 부드럽습니다.
    SpringArm->TargetOffset = FMath::VInterpTo(SpringArm->TargetOffset, Final, DeltaTime, 100.f);
}

void UCameraFunctionLibrary::UpdateTopDownCameraZoomEase(
    const UObject* WorldContextObject,
    USpringArmComponent* SpringArm,
    AActor* ReferenceActor,
    float DesiredArmLength,
    FRotator DesiredRotation,
    FCameraZoomLerpState& InOutState,
    float InterpSpeed,
    float CollisionMargin)
{
    if (!WorldContextObject || !SpringArm || !ReferenceActor) return;
    const UWorld* World = WorldContextObject->GetWorld();
    if (!World) return;

    const float DeltaTime = World->GetDeltaSeconds();

    // 초기 프레임: SpringArm의 현재 값으로 시드
    if (!InOutState.bInitialized)
    {
        InOutState.CurrentArmLength = SpringArm->TargetArmLength;
        InOutState.CurrentRotation  = SpringArm->GetRelativeRotation();
        InOutState.bInitialized     = true;
    }

    // Arm Length와 Rotation을 동시에 목표값으로 부드럽게 보간 (벽과 무관한 "이상값")
    InOutState.CurrentArmLength = FMath::FInterpTo(
        InOutState.CurrentArmLength, DesiredArmLength, DeltaTime, InterpSpeed);
    InOutState.CurrentRotation = FMath::RInterpTo(
        InOutState.CurrentRotation, DesiredRotation, DeltaTime, InterpSpeed);

    // Rotation 먼저 적용 (다음 sweep 방향이 올바르게 나오도록)
    SpringArm->SetRelativeRotation(InOutState.CurrentRotation);

    // 엔진과 같은 피벗(TargetOffset은 월드 좌표, SpringArmComponent.cpp:130)에서 카메라 뒤 방향으로 검사
    const FVector OriginWorld = SpringArm->GetComponentLocation() + SpringArm->TargetOffset;
    const FVector BackDir  = -SpringArm->GetForwardVector();
    const FVector TraceEnd = OriginWorld + BackDir * InOutState.CurrentArmLength;

    float ClampedArm = InOutState.CurrentArmLength;

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(ReferenceActor);

    // 카메라가 지나갈 길만 얇은 구(반지름 = 여유 거리)로 검사한다. ProbeSize(60)를 쓰면 옆 벽 문틀처럼 화면을 안 가리는 것에 걸려 줌인된다.
    // 래그돌 중엔 피벗(골반)이 벽에 붙을 수 있어서, 처음부터 겹친 벽은 거리 0으로 오지 않게 뺀다
    QueryParams.bFindInitialOverlaps = false;
    if (World->SweepSingleByChannel(HitResult, OriginWorld, TraceEnd, FQuat::Identity,
        SpringArm->ProbeChannel, FCollisionShape::MakeSphere(FMath::Max(0.f, CollisionMargin)), QueryParams))
    {
        ClampedArm = FMath::Min(ClampedArm, HitResult.Distance);
    }

    // 이상값은 InOutState에 보존 → 벽이 치워지면 자연스럽게 원하는 거리로 복귀
    SpringArm->TargetArmLength = ClampedArm;
}
