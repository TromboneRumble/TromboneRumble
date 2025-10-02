// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DefaultCharacterController.generated.h"

class UInteractorComponent;
class UPT_UIInGame;
class UInputMappingContext;
class UInputAction;
class ADefaultTromboneCharacter;

/**
 * 
 */
UCLASS()
class TROMBONERUMBLE_API ADefaultCharacterController : public APlayerController
{
	GENERATED_BODY()
public:
	ADefaultCharacterController();
	void ShowInteractionUI(bool bShow) const;

	// InputActions
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> JumpAction;
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> LookAction;
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> InteractAction;
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> TackleAction;
	// ~InputActions

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaSeconds) override;

private:

	// Input handlers
	void Handle_Move(const struct FInputActionValue& Value);
	void Handle_Look(const struct FInputActionValue& Value);
	void Handle_JumpPressed();
	void Handle_JumpReleased();
	void Handle_Interact();
	void Handle_Tackle();
	// ~Input handlers


	UPROPERTY(Transient)
	TObjectPtr<ADefaultTromboneCharacter> CachedOwnerCharacter = nullptr;

	TSubclassOf<UUserWidget> UIInGameClass;

	UPROPERTY(Transient)
	TObjectPtr<UPT_UIInGame> UIInGame;
};
