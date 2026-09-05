#include "Framework/PlayerController/CustomizePlayerController.h"
#include "Subsystems/TromboneUISubsystem.h"
#include "Camera/CameraActor.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "UObject/UObjectIterator.h"

ACustomizePlayerController::ACustomizePlayerController()
{
	bShowMouseCursor = true;
	PrimaryActorTick.bCanEverTick = true;
}

void ACustomizePlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (UTromboneUISubsystem* UISubsystem = UTromboneUISubsystem::Get(this))
	{
		UISubsystem->AttachRootUI(this);
	}
#if !UE_BUILD_SHIPPING
	if (!CheatManager)
		AddCheats(true);
#endif
}

void ACustomizePlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void ACustomizePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (CustomizeMouseHeldAction)
		{
			EIC->BindAction(CustomizeMouseHeldAction, ETriggerEvent::Started,   this, &ThisClass::Handle_CustomizeMouseHeldStart);
			EIC->BindAction(CustomizeMouseHeldAction, ETriggerEvent::Completed, this, &ThisClass::Handle_CustomizeMouseHeldEnd);
		}
		if (CustomizeRotateAction)
		{
			EIC->BindAction(CustomizeRotateAction, ETriggerEvent::Triggered, this, &ThisClass::Handle_CustomizeRotate);
		}
	}

	// SetupInputComponent는 InitInputSystem 경로에서 보장되어 실행되므로(레벨 전환 후 재초기화 포함)
	// 매핑 컨텍스트도 여기서 바인딩과 함께 등록해 travel 후에도 확실히 활성화되도록 한다.
	if (const ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Sub = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (CustomizeMappingContext)
				Sub->AddMappingContext(CustomizeMappingContext, 0);
		}
	}
}

void ACustomizePlayerController::AutoManageActiveCameraTarget(AActor* SuggestedTarget)
{
	for (TObjectIterator<ACameraActor> It; It; ++It)
	{
		if (ACameraActor* Cam = *It)
		{
			if (Cam->GetWorld() != GetWorld()) continue;
			SuggestedTarget = Cam;
			break;
		}
	}
	Super::AutoManageActiveCameraTarget(SuggestedTarget);
}

void ACustomizePlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	APawn* OwnedPawn = GetPawn();
	if (!OwnedPawn) return;

	if (bMouseHeld)
	{
		// 목표 속도로 부드럽게 수렴(뻣뻣함 완화)
		RotationVelocity = FMath::FInterpTo(RotationVelocity, TargetRotationVelocity, DeltaTime, RotationSmoothingSpeed);
		// 드래그를 멈추면(Triggered 미발생) 목표가 0으로 감쇠 → 회전 정지
		TargetRotationVelocity = FMath::FInterpTo(TargetRotationVelocity, 0.f, DeltaTime, RotationSmoothingSpeed);
	}
	else
	{
		// 손을 떼면 관성 감속
		RotationVelocity = FMath::FInterpTo(RotationVelocity, 0.f, DeltaTime, DecelerationRate);
		if (FMath::Abs(RotationVelocity) <= MinVelocityThreshold)
			RotationVelocity = 0.f;
	}

	if (FMath::Abs(RotationVelocity) > MinVelocityThreshold)
		OwnedPawn->AddActorWorldRotation(FRotator(0.f, RotationVelocity, 0.f));
}

void ACustomizePlayerController::Handle_CustomizeMouseHeldStart()
{
	bMouseHeld = true;
}

void ACustomizePlayerController::Handle_CustomizeMouseHeldEnd()
{
	bMouseHeld = false;
}

void ACustomizePlayerController::Handle_CustomizeRotate(const FInputActionValue& Value)
{
	if (!bMouseHeld) return;

	const float Delta = Value.Get<float>();
	TargetRotationVelocity = -Delta * RotationSensitivity;   // 부호 반전(방향 교정) + 감도
}
