// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UserWidgets/InGame/InGameSpeakerWidget.h"
#include "Components/ActorComponents/TromboneVOIPTalker.h"

void UInGameSpeakerWidget::Init(UTromboneVOIPTalker* InTalker)
{
	WeakTalker = InTalker;
	BindToTalker();
}

void UInGameSpeakerWidget::NativeConstruct()
{
	Super::NativeConstruct();
	// 스크린 위젯 컴포넌트는 화면 add/remove마다 NativeDestruct/NativeConstruct 된다.
	// 매 재구성 시 재바인딩해야 1회성 Init 바인딩이 churn으로 사라지지 않는다.
	BindToTalker();
}

void UInGameSpeakerWidget::NativeDestruct()
{
	if (WeakTalker.IsValid())
	{
		WeakTalker->OnTalkingStateChanged.RemoveDynamic(this, &ThisClass::HandleTalkingStateChanged);
	}
	// WeakTalker는 리셋하지 않는다 — 다음 NativeConstruct에서 재바인딩에 사용.
	Super::NativeDestruct();
}

void UInGameSpeakerWidget::BindToTalker()
{
	UTromboneVOIPTalker* Talker = WeakTalker.Get();
	if (!Talker)
	{
		ApplyVisible(false);
		return;
	}

	// 중복 등록 방지 후 재등록.
	Talker->OnTalkingStateChanged.RemoveDynamic(this, &ThisClass::HandleTalkingStateChanged);
	Talker->OnTalkingStateChanged.AddDynamic(this, &ThisClass::HandleTalkingStateChanged);
	// off-screen 동안 놓친 broadcast를 현재 상태로 자가 보정.
	ApplyVisible(Talker->IsSpeaking());
}

void UInGameSpeakerWidget::HandleTalkingStateChanged(bool bIsTalking)
{
	ApplyVisible(bIsTalking);
}

void UInGameSpeakerWidget::ApplyVisible(bool bIsTalking)
{
	// 스크린 스페이스 위젯 컴포넌트에서 Collapsed 토글은 레이어 슬롯이 0 크기로 접혀 다시 펼쳐지지 않는다.
	// 가시성은 항상 HitTestInvisible로 두고 RenderOpacity로 show/hide (git 36c1b447의 검증된 방식).
	SetVisibility(ESlateVisibility::HitTestInvisible);
	SetRenderOpacity(bIsTalking ? 1.0f : 0.0f);
}
