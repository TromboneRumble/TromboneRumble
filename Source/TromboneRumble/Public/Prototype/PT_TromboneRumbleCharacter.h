// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "PT_TromboneRumbleCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class APT_Trumpet;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class APT_TromboneRumbleCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APT_TromboneRumbleCharacter();
	
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
	void Server_Interaction(APT_Trumpet* TrumpetToGrab);

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
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	TObjectPtr<UCameraComponent> FollowCamera;
	
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> GrabAction;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> TackleAction;

	float InteractionDistance = 2000.0f;
	
	UPROPERTY()
	TObjectPtr<class APT_Trumpet> FocusedTrumpet = nullptr;

	UPROPERTY()
	TObjectPtr<class APT_Trumpet> HeldTrumpet = nullptr;

	float TackleAnimationDuration = 1.0f;
};