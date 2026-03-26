// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/Rhythm/RhythmLeaderBoard.h"
#include "Framework/DefaultPlayerState.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "framework/InGameState.h"
#include "GameFramework/PlayerState.h"
#include "UI/UserWidgets/Rhythm/RhythmLeaderBoardEntry.h"
#include "Characters/DefaultPlayerController.h"

void URhythmLeaderBoard::NativeConstruct()
{
	Super::NativeConstruct();
	if (IsDesignTime()) return;
	

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

	// UI가 아직 화면에 그려지지 않아 높이를 알 수 없는 경우
	if (Canvas_LeaderBoard->GetCachedGeometry().GetLocalSize().Y <= 0.f)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &URhythmLeaderBoard::RefreshLeaderboard, UpdatedPlayerState));
		}
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	AInGameState* InGameState = World->GetGameState<AInGameState>();
	if (!InGameState) return;

	UpdateRowHeight();

	// Score 기준 내림차순 정렬
	TArray<APlayerState*> Players;
	InGameState->GetPlayersSortedByScore(Players);

	if (Players.Num() > MaxVisibleRows)
	{
		Players.SetNum(MaxVisibleRows);
	}

	if (Players.Num() == 0) return;

	const float AdditionalPadding = RowHeight * 0.1f;
	const float EntryHeight = RowHeight * 0.9f;
	const float Spacing = EntryHeight + AdditionalPadding;

	// 목표 Y 좌표들을 저장할 배열
	TArray<float> TargetYPositions;
	TargetYPositions.SetNum(Players.Num());

	// 등수대로 고정된 슬롯에 배치
	if (LeaderboardStyle == ELeaderboardStyle::FixedSlot)
	{
		
		for (int32 i = 0; i < Players.Num(); ++i)
		{
			TargetYPositions[i] = i * Spacing;
		}
	}
	// 점수 비례 방식
	else if (LeaderboardStyle == ELeaderboardStyle::Proportional)
	{
		
		const float MaxScore = Players[0]->GetScore();
		const float MinScore = Players.Last()->GetScore();

		// UI 초기화 단계에서 GetCachedGeometry()가 0일 수 있으므로 최소 높이를 보장
		const float CanvasHeight = FMath::Max(Canvas_LeaderBoard->GetCachedGeometry().GetLocalSize().Y, RowHeight * MaxVisibleRows);
		float EntryWidgetHeight = RowHeight; // 기본 백업값
		if (EntryClass)
		{
			EntryWidgetHeight = EntryClass->GetDefaultObject<URhythmLeaderBoardEntry>()->GetDesiredSize().Y;

			if (EntryWidgetHeight <= 0.f)
			{
				EntryWidgetHeight = 73.f; // EntryClass의 실제 높이를 상수로 정함
			}
		}

		const float MinDistance = EntryWidgetHeight;
		const float MinY = 0.f;
		const float MaxY = FMath::Max(0.f, CanvasHeight - EntryWidgetHeight);

		if (MaxScore == 0.f && MinScore == 0.f)
		{
			// 모두 0점인 상황 (게임 시작 직후) : 현재 플레이어 수에 맞춰 양끝 정렬(Space Between) 배치
			for (int32 i = 0; i < Players.Num(); ++i)
			{
				if (Players.Num() == 1)
				{
					TargetYPositions[i] = MinY;
				}
				else
				{
					TargetYPositions[i] = FMath::Lerp(MinY, MaxY, static_cast<float>(i) / (Players.Num() - 1));
				}
			}
		}
		else
		{
			// 점수 변동이 생긴 이후 점수에 비례하여 위치 계산
			const float ScoreRange = FMath::Max(1.0f, MaxScore - MinScore);

			for (int32 i = 0; i < Players.Num(); ++i)
			{
				const float PlayerScore = Players[i]->GetScore();
				const float T = (MaxScore - PlayerScore) / ScoreRange;
				TargetYPositions[i] = FMath::Lerp(MinY, MaxY, T);
			}

			// 겹침 방지 (위에서 아래로 밀어내기 - 동점자 처리용)
			for (int32 i = 1; i < Players.Num(); ++i)
			{
				if (TargetYPositions[i] < TargetYPositions[i - 1] + MinDistance)
				{
					TargetYPositions[i] = TargetYPositions[i - 1] + MinDistance;
				}
			}
			// 밀어내다 캔버스를 벗어나면 아래에서 위로 끌어올리기
			if (TargetYPositions.Last() > MaxY)
			{
				TargetYPositions.Last() = MaxY;
				for (int32 i = Players.Num() - 2; i >= 0; --i)
				{
					if (TargetYPositions[i] > TargetYPositions[i + 1] - MinDistance)
					{
						TargetYPositions[i] = TargetYPositions[i + 1] - MinDistance;
					}
				}
			}
		}
	}

	ADefaultPlayerState* LocalPS = LocalPlayerState.Get();
	int32 Rank = 1;

	// 위젯 생성 및 갱신
	for (int32 i = 0; i < Players.Num(); ++i)
	{
		APlayerState* PS = Players[i];
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

			// CanvasPanel에 붙이기
			if (UCanvasPanelSlot* CanvasSlot = Canvas_LeaderBoard->AddChildToCanvas(Entry))
			{
				CanvasSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f)); // 좌상단 기준
				CanvasSlot->SetAlignment(FVector2D(0.f, 0.f));
				CanvasSlot->SetAutoSize(true);

				// 최초 생성 시 목표 위치로 즉시 이동
				CanvasSlot->SetPosition(FVector2D(0.f, TargetYPositions[i]));
			}

			EntryMap.Add(PS, Entry);
		}

		Entry->SetPlayerName(PS->GetPlayerName());

		const bool bIsLocal = (LocalPS && PS == LocalPS);

		FLinearColor SkinColor = FLinearColor{ 1.0f,0.f,1.0f,1.f };
		if (ADefaultPlayerState* DefaultPlayerState = Cast<ADefaultPlayerState>(PS))
		{
			SkinColor = DefaultPlayerState->GetSkinColor();
		}

		Entry->UpdateData(
			SkinColor,
			static_cast<int32>(PS->GetScore()),
			bIsLocal);

		// 계산된 목표 위치(Y)를 전달
		Entry->SetTargetY(TargetYPositions[i]);

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
