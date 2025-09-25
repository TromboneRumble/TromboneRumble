// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "TromboneRumbleCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ATromboneRumbleCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATromboneRumbleCharacter();
	
	void Drop();

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;
	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Interaction();

	UFUNCTION(Server, Reliable)
	void Server_Interaction(class ATrumpet* TrumpetToGrab);

	void Tackle();

	UFUNCTION(Server, Reliable)
	void Server_Tackle();

	UFUNCTION(Server, Reliable)
	void Server_Drop();

private:
	void CheckForInteraction();
	void EndTackleAnimation();

public:
    UPROPERTY(Replicated, BlueprintReadOnly)
	bool bIsTackling = false;

	UFUNCTION(BlueprintCallable)
	bool GetIsTackling() const { return bIsTackling; }

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* FollowCamera;
	
	UPROPERTY(EditAnywhere, Category = Input)
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> GrabAction;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> TackleAction;

	float InteractionDistance = 2000.0f;
	
	UPROPERTY()
	TObjectPtr<class ATrumpet> FocusedTrumpet = nullptr;

	UPROPERTY()
	TObjectPtr<class ATrumpet> HeldTrumpet = nullptr;

	float TackleAnimationDuration = 1.0f;
};