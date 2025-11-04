// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Utilities/Defines.h"
#include "DefaultPlayerController.generated.h"

class UAkAudioEvent;
class UInteractorComponent;
class UPT_UIInGame;
class UInputMappingContext;
class UInputAction;
class ADefaultTromboneCharacter;

UCLASS()
class TROMBONERUMBLE_API ADefaultPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	ADefaultPlayerController();
	void ShowInteractionUI(bool bShow) const;

	// InputActions
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> JumpAction;
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> InteractAction;
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> SprintAction;
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> AttackAction;
	// ~InputActions

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void AcknowledgePossession(APawn* InPawn) override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	// Input handlers
	void Handle_Move(const struct FInputActionValue& Value);
	void Handle_JumpPressed();
	void Handle_JumpReleased();
	void Handle_Interact();
	void Handle_SprintPressed();
	void Handle_SprintReleased();
	void Handle_Attack();
	// ~Input handlers

	EGameState GetGameState() const;

	// UI
	void InitializeUI();
	void InitializeLobbyUI();
	void InitializeInGameUI();
	// ~UI

	UFUNCTION(Server, Reliable)
	void Server_NotifyClientReady();

	TSubclassOf<UUserWidget> InGameUIClass;

	UPROPERTY(Transient)
	TObjectPtr<UPT_UIInGame> InGameUI;

	UPROPERTY(Transient)
	TObjectPtr<ADefaultTromboneCharacter> CachedOwnerCharacter = nullptr;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAkAudioEvent> TestSoundEvent;

	bool bIsSprinting = false;
};
