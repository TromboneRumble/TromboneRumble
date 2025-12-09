// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Crown.h"
#include "Framework/InGameState.h"
#include "Framework/DefaultPlayerState.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Utilities/DebugHelper.h"

ACrown::ACrown()
{
	PrimaryActorTick.bCanEverTick = false;

}

void ACrown::BeginPlay()
{
	Super::BeginPlay();
	if (UWorld* World = GetWorld())
	{
		if (AInGameState* InGameState = World->GetGameState<AInGameState>())
		{
			InGameState->OnLeaderChanged.AddDynamic(this, &ThisClass::HandleLeaderChanged);
		}
		World->GetTimerManager().SetTimer(
			InitialLeaderTimerHandle,
			this,
			&ThisClass::TryAttachToInitialLeader,
			0.1f,    
			true,    
			0.0f     
		);
	}
}

void ACrown::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (AInGameState* InGameState = World->GetGameState<AInGameState>())
		{
			InGameState->OnLeaderChanged.RemoveDynamic(this, &ThisClass::HandleLeaderChanged);
		}
	}
	Super::EndPlay(EndPlayReason);
}

void ACrown::HandleLeaderChanged(APlayerState* NewLeader, APlayerState* OldLeader)
{
	// 기존 Attach 해제
	if (AttachedCharacter.IsValid())
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		AttachedCharacter = nullptr;
	}

	if (!NewLeader)
	{
		return;
	}

	if (APawn* Pawn = NewLeader->GetPawn())
	{
		if (ADefaultTromboneCharacter* Char = Cast<ADefaultTromboneCharacter>(Pawn))
		{
			AttachedCharacter = Char;

			FAttachmentTransformRules Rules(EAttachmentRule::SnapToTarget, true);
			AttachToComponent(Char->GetMesh(), Rules, TEXT("socket_crown"));
		}
	}
}

void ACrown::TryAttachToInitialLeader()
{
	UWorld* World = GetWorld();
	if (!World) return;

	AInGameState* InGameState = World->GetGameState<AInGameState>();
	if (!InGameState)
	{
		return;
	}

	if (APlayerState* LeaderPS = InGameState->GetCurrentLeader())
	{
		if (APawn* Pawn = LeaderPS->GetPawn())
		{
			if (ADefaultTromboneCharacter* Char = Cast<ADefaultTromboneCharacter>(Pawn))
			{
				HandleLeaderChanged(LeaderPS, nullptr);

				// 한 번 성공했으면 더 이상 타이머 돌릴 필요 없음
				World->GetTimerManager().ClearTimer(InitialLeaderTimerHandle);
				return;
			}
		}
	}
}



