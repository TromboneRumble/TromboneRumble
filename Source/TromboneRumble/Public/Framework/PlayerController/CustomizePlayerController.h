#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CustomizePlayerController.generated.h"

struct FInputActionValue;
class UInputAction;
class UInputMappingContext;

UCLASS()
class TROMBONERUMBLE_API ACustomizePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ACustomizePlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void AutoManageActiveCameraTarget(AActor* SuggestedTarget) override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void ReceivedPlayer() override;

private:
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputMappingContext> CustomizeMappingContext;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> CustomizeMouseHeldAction;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> CustomizeRotateAction;

	void Handle_CustomizeMouseHeldStart();
	void Handle_CustomizeMouseHeldEnd();
	void Handle_CustomizeRotate(const FInputActionValue& Value);

	bool bMouseHeld = false;
	float RotationVelocity = 0.f;
	float TargetRotationVelocity = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Customize|Rotation")
	float RotationSensitivity = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Customize|Rotation")
	float RotationSmoothingSpeed = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Customize|Rotation")
	float DecelerationRate = 3.f;

	static constexpr float MinVelocityThreshold = 0.05f;
};
