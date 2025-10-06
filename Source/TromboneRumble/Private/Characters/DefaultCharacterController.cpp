// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/DefaultCharacterController.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "ProtoType/PT_UIInGame.h"
#include "Blueprint/UserWidget.h"

ADefaultCharacterController::ADefaultCharacterController()
{
	//TODO : 하드 레퍼런싱에서 datatable로 변경
	static ConstructorHelpers::FClassFinder<UUserWidget> InGameWidgetClassFinder(TEXT("/Game/Blueprints/Prototype/WBP_PT_InGame.WBP_PT_InGame_C"));
	if (InGameWidgetClassFinder.Succeeded())
	{
		UIInGameClass = InGameWidgetClassFinder.Class;
	}
}

void ADefaultCharacterController::ShowInteractionUI(bool bShow) const
{
	if (!UIInGame) return;

	UIInGame->ShowInteractionHint(bShow);
}

void ADefaultCharacterController::BeginPlay()
{
	Super::BeginPlay();
	if (UIInGameClass)
	{
		UIInGame = CreateWidget<UPT_UIInGame>(GetWorld(), UIInGameClass);
		if (UIInGame)
		{
			UIInGame->AddToViewport();
			const FInputModeGameOnly InputModeData;
			SetInputMode(InputModeData);
			bShowMouseCursor = false;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create UIInGame"));
		}
	}
}

void ADefaultCharacterController::OnPossess(APawn* APawn)
{
	Super::OnPossess(APawn);
	CachedOwnerCharacter = Cast<ADefaultTromboneCharacter>(APawn);
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext,0);
			}
		}
	}
}

void ADefaultCharacterController::OnUnPossess()
{
	Super::OnUnPossess();
	CachedOwnerCharacter = nullptr;
}

void ADefaultCharacterController::AcknowledgePossession(APawn* InPawn)
{
	Super::AcknowledgePossession(InPawn);
	CachedOwnerCharacter = Cast<ADefaultTromboneCharacter>(InPawn);
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (auto* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

void ADefaultCharacterController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (MoveAction)    EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Handle_Move);
		if (LookAction)    EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Handle_Look);
		if (InteractAction)EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &ThisClass::Handle_Interact);
		if (TackleAction)  EIC->BindAction(TackleAction, ETriggerEvent::Started, this, &ThisClass::Handle_Tackle);
		if (JumpAction)    EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ThisClass::Handle_JumpPressed);
		if (JumpAction)    EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ThisClass::Handle_JumpReleased);
	}
}

void ADefaultCharacterController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ADefaultCharacterController::Handle_Move(const struct FInputActionValue& Value)
{
	if (CachedOwnerCharacter) CachedOwnerCharacter->Move(Value);
}

void ADefaultCharacterController::Handle_Look(const struct FInputActionValue& Value)
{
	if (CachedOwnerCharacter) CachedOwnerCharacter->Look(Value);
}

void ADefaultCharacterController::Handle_JumpPressed()
{
	if (CachedOwnerCharacter) CachedOwnerCharacter->Jump();
}

void ADefaultCharacterController::Handle_JumpReleased()
{
	if (CachedOwnerCharacter) CachedOwnerCharacter->StopJumping();
}

void ADefaultCharacterController::Handle_Interact()
{
	if (CachedOwnerCharacter) CachedOwnerCharacter->Interact();
}

void ADefaultCharacterController::Handle_Tackle()
{
	if (CachedOwnerCharacter) CachedOwnerCharacter->Tackle();
}