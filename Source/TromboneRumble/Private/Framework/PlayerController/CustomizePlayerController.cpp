#include "Framework/PlayerController/CustomizePlayerController.h"
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
#if !UE_BUILD_SHIPPING
	if (!CheatManager)
		AddCheats(true);
#endif
}

void ACustomizePlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (!IsLocalController()) return;
	if (const ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Sub = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (CustomizeMappingContext)
				Sub->AddMappingContext(CustomizeMappingContext, 0);
		}
	}
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

	if (!bMouseHeld)
	{
		if (FMath::Abs(RotationVelocity) > MinVelocityThreshold)
		{
			if (APawn* OwnedPawn = GetPawn())
				OwnedPawn->AddActorWorldRotation(FRotator(0.f, RotationVelocity, 0.f));
			RotationVelocity = FMath::FInterpTo(RotationVelocity, 0.f, DeltaTime, DecelerationRate);
		}
		else
		{
			RotationVelocity = 0.f;
		}
	}
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
	RotationVelocity = Delta;
	if (APawn* OwnedPawn = GetPawn())
		OwnedPawn->AddActorWorldRotation(FRotator(0.f, Delta, 0.f));
}
