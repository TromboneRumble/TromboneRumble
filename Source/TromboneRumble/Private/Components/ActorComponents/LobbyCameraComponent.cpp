// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Components/ActorComponents/LobbyCameraComponent.h"
#include "EngineUtils.h"
#include "Camera/CameraActor.h"
#include "Framework/LobbyGameState.h"
#include "GameFramework/PlayerController.h"
#include "Utilities/DebugHelper.h"

ULobbyCameraComponent::ULobbyCameraComponent()
{
	// Ticks only while the intro camera is running
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void ULobbyCameraComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}
	StopDirecting();

	Super::EndPlay(EndPlayReason);
}

void ULobbyCameraComponent::BeginPlay()
{
	Super::BeginPlay();

	TryStartDirecting(0);
}

void ULobbyCameraComponent::TryStartDirecting(const int32 RetryCount)
{
	OwnerPC = Cast<APlayerController>(GetOwner());
	if (!OwnerPC)
	{
		return;
	}

	// A remote player's controller on a listen server gets a net connection as its Player - never local
	if (OwnerPC->Player && !OwnerPC->IsLocalController())
	{
		return;
	}

	// Claim the camera first and judge from the replicated lobby state later: the lobby camera actor
	// is baked into the map, so it is the only lobby signal available on the very first frame.
	// Waiting for any replicated data here would show the hidden pawn until that data arrives
	CenterCamera = FindCenterCamera();
	if (!CenterCamera || !OwnerPC->IsLocalController())
	{
		// Giving up silently is expected on non-lobby levels: they have no tagged camera
		if (RetryCount < MaxRetryCount)
		{
			GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this,
				[this, RetryCount] { TryStartDirecting(RetryCount + 1); }));
		}
		return;
	}

	bDirecting = true;
	SetComponentTickEnabled(true);
	OwnerPC->SetViewTarget(CenterCamera);
	LOG_WITH_CURRENT_CONTEXT(Log, FString::Printf(TEXT("Lobby intro camera started. (retries: %d)"), RetryCount));

	ResolveLobbyState(0);
}

void ULobbyCameraComponent::ResolveLobbyState(const int32 RetryCount)
{
	if (!bDirecting)
	{
		return;
	}

	LobbyGameState = GetWorld()->GetGameState<ALobbyGameState>();
	if (!LobbyGameState)
	{
		if (RetryCount < MaxRetryCount)
		{
			GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this,
				[this, RetryCount] { ResolveLobbyState(RetryCount + 1); }));
			return;
		}

		// Holding the lobby camera without a lobby flow would leave the player stuck watching it
		LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("LobbyGameState not found. Handing the camera back to the pawn."));
		BlendToOwnPawn(0);
		return;
	}

	// Evaluate the current state directly, since OnRep may have fired before we subscribed.
	// Players who join after the intro is over blend straight to their own character
	if (!IsPreGroundedState(LobbyGameState->GetCurrentLobbyState()))
	{
		BlendToOwnPawn(0);
		return;
	}

	LobbyGameState->OnLobbyStateChanged.AddUniqueDynamic(this, &ThisClass::HandleLobbyStateChanged);
}

void ULobbyCameraComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bDirecting || !OwnerPC || !CenterCamera)
	{
		return;
	}

	// Keep the lobby camera as the view target every frame: possession and the server's
	// ClientSetViewTarget RPC can overwrite it at any time on the client
	if (OwnerPC->GetViewTarget() != CenterCamera)
	{
		OwnerPC->SetViewTarget(CenterCamera);
	}
}

AActor* ULobbyCameraComponent::GetViewTargetOverride()
{
	if (bIntroFinished)
	{
		return nullptr;
	}

	// Once the lobby flow is past the falling phase, never override.
	// This also covers the server-side calls made for remote players' controllers
	if (const ALobbyGameState* CurrentLobbyGameState = GetWorld()->GetGameState<ALobbyGameState>())
	{
		if (!IsPreGroundedState(CurrentLobbyGameState->GetCurrentLobbyState()))
		{
			return nullptr;
		}
	}

	if (!CenterCamera)
	{
		CenterCamera = FindCenterCamera();
	}
	return CenterCamera;
}

void ULobbyCameraComponent::BlendToOwnPawn(const int32 RetryCount)
{
	APawn* MyPawn = OwnerPC ? OwnerPC->GetPawn() : nullptr;
	if (!MyPawn)
	{
		if (RetryCount < MaxRetryCount)
		{
			GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this,
				[this, RetryCount] { BlendToOwnPawn(RetryCount + 1); }));
		}
		else
		{
			LOG_WITH_CURRENT_CONTEXT(Warning, TEXT("Failed to blend to own pawn: no pawn."));
			StopDirecting();
		}
		return;
	}

	OwnerPC->SetViewTargetWithBlend(MyPawn, BlendTime, VTBlend_Cubic);
	LOG_WITH_CURRENT_CONTEXT(Log, TEXT("Lobby intro camera finished. Blending to own pawn."));
	StopDirecting();
}

void ULobbyCameraComponent::StopDirecting()
{
	bDirecting = false;
	bIntroFinished = true;
	SetComponentTickEnabled(false);

	if (LobbyGameState)
	{
		LobbyGameState->OnLobbyStateChanged.RemoveDynamic(this, &ThisClass::HandleLobbyStateChanged);
	}
}

ACameraActor* ULobbyCameraComponent::FindCenterCamera() const
{
	for (TActorIterator<ACameraActor> It(GetWorld()); It; ++It)
	{
		if (ACameraActor* CameraActor = *It; CameraActor && CameraActor->ActorHasTag(CameraActorTag))
		{
			return CameraActor;
		}
	}
	return nullptr;
}

bool ULobbyCameraComponent::IsPreGroundedState(const ELobbyState State)
{
	// CountdownToStandup 진입 = 서버의 전원 접지 판정 완료 (PollAllGrounded의 강제 타임아웃 포함)
	return State == ELobbyState::None
		|| State == ELobbyState::WaitingForPlayers
		|| State == ELobbyState::FallingPlayers;
}

void ULobbyCameraComponent::HandleLobbyStateChanged(const ELobbyState NewState)
{
	if (!bDirecting || IsPreGroundedState(NewState))
	{
		return;
	}

	BlendToOwnPawn(0);
}
