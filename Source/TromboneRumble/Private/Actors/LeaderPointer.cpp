// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/LeaderPointer.h"
#include "Framework/InGameState.h"
#include "Framework/DefaultPlayerState.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Utilities/DebugHelper.h"

ALeaderPointer::ALeaderPointer()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ALeaderPointer::BeginPlay()
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
	BaseRelativeLocation = GetRootComponent()->GetRelativeLocation();
}

void ALeaderPointer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	float Time = GetWorld()->GetTimeSeconds();
	float ZOffset = FMath::Sin(Time * FloatSpeed) * FloatHeight;
	FVector NewLocation = BaseRelativeLocation;
	NewLocation.Z += ZOffset;

	SetActorRelativeLocation(NewLocation);

	FRotator DeltaRotation = FRotator(0.0f, RotationSpeed * DeltaTime, 0.0f);

	AddActorLocalRotation(DeltaRotation);
}

void ALeaderPointer::EndPlay(const EEndPlayReason::Type EndPlayReason)
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

void ALeaderPointer::HandleLeaderChanged(APlayerState* NewLeader, APlayerState* OldLeader)
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
			USceneComponent* TargetComp = Char->GetRootComponent();
			GetRootComponent()->SetAbsolute(false, true, false);

			FAttachmentTransformRules Rules(EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, true);
			AttachToComponent(TargetComp, Rules);


			if (AttachedCharacter->IsLocallyControlled())
			{
				GetRootComponent()->SetVisibility(false, true);
				SetActorRelativeLocation(LocalPlayerDistanceOffset);
			}
			else
			{
				GetRootComponent()->SetVisibility(true, true);
				SetActorRelativeLocation(OtherPlayerDistanceOffset);
			}
			BaseRelativeLocation = GetRootComponent()->GetRelativeLocation();
		}
	}
}

void ALeaderPointer::TryAttachToInitialLeader()
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

