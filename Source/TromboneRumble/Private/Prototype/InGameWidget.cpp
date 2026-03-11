// Fill out your copyright notice in the Description page of Project Settings.

#include "ProtoType/InGameWidget.h"
#include "Components/Image.h"
#include "Framework/InGameState.h"
#include "Utilities/DebugHelper.h"

void UInGameWidget::ToggleGuideUI()
{
	const ESlateVisibility CurrentVisibility = Image_Guide->GetVisibility();
	const ESlateVisibility TargetVisibility = CurrentVisibility == ESlateVisibility::Visible ? ESlateVisibility::Collapsed : ESlateVisibility::Visible;
	
	Image_Guide->SetVisibility(TargetVisibility);
}

void UInGameWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (UWorld* World = GetWorld())
	{
		// GameState가 이미 존재하면 즉시 바인딩
		if (AInGameState* GameState = Cast<AInGameState>(World->GetGameState()))
		{
			BindToInGameState(GameState);
		}
		else
		{
			// 멀티플레이 클라이언트 환경에서 GameState가 아직 복제되지 않았다면 이벤트 대기
			World->GameStateSetEvent.AddUObject(this, &ThisClass::BindToInGameState);
		}
	}
}

void UInGameWidget::BindToInGameState(AGameStateBase* NewGameState)
{
	if (AInGameState* GameState = Cast<AInGameState>(NewGameState))
	{
		// 중복 바인딩 방지
		GameState->OnInGameStateChanged.RemoveDynamic(this, &ThisClass::HandleInGameStateChanged);
		GameState->OnInGameStateChanged.AddDynamic(this, &ThisClass::HandleInGameStateChanged);

		EInGameState CurrentState = GameState->GetCurrentGameState();

		// 혹시 바인딩 시점에 이미 게임이 종료된 상태라면 즉시 꺼지도록 처리
		if (GameState->GetCurrentGameState() == EInGameState::End)
		{
			HandleInGameStateChanged(EInGameState::End);
		}
	}
	
}

void UInGameWidget::HandleInGameStateChanged(EInGameState NewState)
{
	if (NewState == EInGameState::End)
	{
		DeactivateWidget();
		RemoveFromParent();
	}
}
