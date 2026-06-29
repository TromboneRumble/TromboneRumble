// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Crown.h"
#include "Framework/InGameState.h"
#include "Framework/DefaultPlayerState.h"
#include "Characters/DefaultTromboneCharacter.h"

ACrown::ACrown()
{
	PrimaryActorTick.bCanEverTick = true;

}

void ACrown::BeginPlay()
{
	Super::BeginPlay();

	// 1등이 정해지기 전(전원 0점)에는 스폰 위치에 보이지 않도록 숨김
	SetActorHiddenInGame(true);

	if (UWorld* World = GetWorld())
	{
		if (AInGameState* InGameState = World->GetGameState<AInGameState>())
		{
			InGameState->OnLeaderChanged.AddDynamic(this, &ThisClass::HandleLeaderChanged);
		}
		//게임 시작했을때 0점인 상태인 사람에게도 왕관 붙히고 싶으면 Timer 쓰기
		/*World->GetTimerManager().SetTimer(
			InitialLeaderTimerHandle,
			this,
			&ThisClass::TryAttachToInitialLeader,
			0.1f,    
			true,    
			0.0f     
		);*/
	}
}

void ACrown::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		GetWorldTimerManager().ClearTimer(InitialLeaderTimerHandle);
		InitialLeaderTimerHandle.Invalidate();

		if (AInGameState* InGameState = World->GetGameState<AInGameState>())
		{
			InGameState->OnLeaderChanged.RemoveDynamic(this, &ThisClass::HandleLeaderChanged);
		}
	}
	
	GetWorldTimerManager().ClearTimer(InitialLeaderTimerHandle);
	
	Super::EndPlay(EndPlayReason);
}

void ACrown::HandleLeaderChanged(APlayerState* NewLeader, APlayerState* OldLeader)
{
	// 부착 방식이 아니라 Tick에서 pelvis 위치를 추적하므로 대상 캐릭터만 갱신한다.
	AttachedCharacter = nullptr;
	BobTime = 0.f;

	if (!NewLeader)
	{
		// 1등이 없으면 숨김 (빈 공간에 남지 않도록)
		SetActorHiddenInGame(true);
		return;
	}

	if (APawn* Pawn = NewLeader->GetPawn())
	{
		if (ADefaultTromboneCharacter* Char = Cast<ADefaultTromboneCharacter>(Pawn))
		{
			AttachedCharacter = Char;
			SetActorHiddenInGame(false);
		}
	}
}

void ACrown::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ACharacter* Char = AttachedCharacter.Get();
	if (!Char || !Char->GetMesh())
	{
		return;
	}

	BobTime += DeltaSeconds;
	const float ZOffset = HoverHeight + BobAmplitude * FMath::Sin(BobTime * BobSpeed);

	// pelvis(몸 중심/물리 루트) 월드 위치 + 월드 수직 오프셋 → 고개 숙임/레그돌과 무관하게 세로축 고정
	const FVector PelvisLoc = Char->GetMesh()->GetSocketLocation(TromboneBones::Pelvis);
	SetActorLocation(PelvisLoc + FVector(0.f, 0.f, ZOffset));
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



