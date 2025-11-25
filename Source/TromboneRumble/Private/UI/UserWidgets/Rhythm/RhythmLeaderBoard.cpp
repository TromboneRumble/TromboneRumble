// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/Rhythm/RhythmLeaderBoard.h"
#include "Framework/DefaultPlayerState.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "framework/InGameState.h"
#include "GameFramework/PlayerState.h"
#include "UI/UserWidgets/Rhythm/RhythmLeaderBoardEntry.h"
#include "Characters/DefaultPlayerController.h"
#include "Utilities/DebugHelper.h"


void URhythmLeaderBoard::NativeConstruct()
{
	Super::NativeConstruct();

	if (UWorld* World = GetWorld())
	{
		if (AInGameState* InGameState = World->GetGameState<AInGameState>())
		{
			InGameState->OnScoreChanged.AddDynamic(this, &ThisClass::RefreshLeaderboard);
		}

		if (APlayerController* PlayerController = GetOwningPlayer())
		{
			if (ADefaultPlayerController* DefaultPlayerController = Cast<ADefaultPlayerController>(PlayerController))
			{
				DefaultPlayerController->OnPlayerStateChanged.AddDynamic(this, &ThisClass::HandleLocalPlayerStateChanged);

				// 이미 PlayerState가 붙어 있는 경우(호스트 등) 즉시 한 번 처리
				if (APlayerState* PS = DefaultPlayerController->PlayerState)
				{
					HandleLocalPlayerStateChanged(PS);
				}
			}
		}
	}

	RefreshLeaderboard(nullptr);
}

void URhythmLeaderBoard::NativeDestruct()
{

	if (UWorld* World = GetWorld())
	{
		if (AInGameState* InGameState = World->GetGameState<AInGameState>())
		{
			InGameState->OnScoreChanged.RemoveDynamic(this, &ThisClass::RefreshLeaderboard);
		}

		if (APlayerController* PlayerController = GetOwningPlayer())
		{
			if (ADefaultPlayerController* DefaultPlayerController = Cast<ADefaultPlayerController>(PlayerController))
			{
				DefaultPlayerController->OnPlayerStateChanged.RemoveDynamic(this, &ThisClass::HandleLocalPlayerStateChanged);
			}
		}
	}

	Super::NativeDestruct();
}

void URhythmLeaderBoard::RefreshLeaderboard(APlayerState* UpdatedPlayerState)
{
	if (!Canvas_LeaderBoard || !EntryClass) return;

	UWorld* World = GetWorld();
	if (!World) return;

	AInGameState* InGameState = World->GetGameState<AInGameState>();
	if (!InGameState) return;

	UpdateRowHeight();

	// Score 기준 내림차순 정렬
	TArray<APlayerState*> Players = InGameState->PlayerArray;	
	Players.Sort([](const APlayerState& A, const APlayerState& B)
		{
			return A.GetScore() > B.GetScore();
		});

	if (Players.Num() > MaxVisibleRows)
	{
		Players.SetNum(MaxVisibleRows);
	}

	ADefaultPlayerState* LocalPS = LocalPlayerState.Get();

	int32 Rank = 1;

	for (APlayerState* PS : Players)
	{
		if (!PS) continue;

		URhythmLeaderBoardEntry* Entry = nullptr;

		// 이미 존재하는 항목이면 재사용
		if (URhythmLeaderBoardEntry** Found = EntryMap.Find(PS))
		{
			Entry = *Found;
		}
		else
		{
			// 새 항목 생성
			Entry = CreateWidget<URhythmLeaderBoardEntry>(World, EntryClass);
			if (!Entry) continue;

			const float AdditionalPadding = RowHeight * 0.1f;
			const float EntryHeight = RowHeight * 0.9f;

			// CanvasPanel에 붙이기
			if (UCanvasPanelSlot* CanvasSlot = Canvas_LeaderBoard->AddChildToCanvas(Entry))
			{
				CanvasSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f)); // 좌상단 기준
				CanvasSlot->SetAlignment(FVector2D(0.f, 0.f));
				CanvasSlot->SetAutoSize(true);

				const float InitialY = (Rank - 1) * (EntryHeight + AdditionalPadding);
				CanvasSlot->SetPosition(FVector2D(0.f, InitialY));
			}

			EntryMap.Add(PS, Entry);
		}

		const bool bIsLocal = (LocalPS && PS == LocalPS);

		// 데이터 갱신 + 목표 랭크 설정
		Entry->SetRowHeight(RowHeight);
		FLinearColor SkinColor = FLinearColor{ 1.0f,0.f,1.0f,1.f };
		if (ADefaultPlayerState* DefaultPlayerState = Cast<ADefaultPlayerState>(PS))
		{
			SkinColor = DefaultPlayerState->GetSkinColor();
		}
		
		Entry->UpdateData(
			SkinColor,
			Rank,
			static_cast<int32>(PS->GetScore()),
			bIsLocal);

		Entry->SetTargetRank(Rank);

		Rank++;
	}

	// 퇴장한 플레이어 + top4 외부 플레이어 처리
	TSet<APlayerState*> VisibleSet(Players);

	for (auto It = EntryMap.CreateIterator(); It; ++It)
	{
		APlayerState* PS = It.Key().Get();

		// PlayerState가 유효하지 않거나, 현재 VisibleSet에 없으면 제거
		if (!PS || !VisibleSet.Contains(PS))
		{
			if (URhythmLeaderBoardEntry* OrphanEntry = It.Value())
			{
				OrphanEntry->RemoveFromParent();
			}

			It.RemoveCurrent();
		}
	}
}

void URhythmLeaderBoard::HandleLocalPlayerStateChanged(APlayerState* NewPlayerState)
{
	if (ADefaultPlayerState* DefaultPS = Cast<ADefaultPlayerState>(NewPlayerState))
	{
		LocalPlayerState = DefaultPS;
		RefreshLeaderboard(DefaultPS);
	}
}

void URhythmLeaderBoard::UpdateRowHeight()
{
	if (!Canvas_LeaderBoard)
	{
		return;
	}

	const FVector2D Size = Canvas_LeaderBoard->GetCachedGeometry().GetLocalSize();
	if (Size.Y > 0.f && MaxVisibleRows > 0)
	{
		RowHeight = Size.Y / static_cast<float>(MaxVisibleRows);
	}
}
