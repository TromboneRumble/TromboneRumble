// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Actors/MatchMenu/MatchMenuBackgroundSwitcher.h"
#include "Framework/GameState/MatchMenuGameState.h"
#include "Kismet/GameplayStatics.h"

void AMatchMenuBackgroundSwitcher::BeginPlay()
{
    Super::BeginPlay();

    if (GetNetMode() == NM_DedicatedServer)
    {
        return;
    }

    // 클라이언트는 BeginPlay 시점에 GameState가 아직 없을 수 있음
    if (AGameStateBase* GS = GetWorld()->GetGameState())
    {
        BindToGameState(GS);
    }
    else
    {
        GetWorld()->GameStateSetEvent.AddUObject(this, &ThisClass::BindToGameState);
    }
}

void AMatchMenuBackgroundSwitcher::BindToGameState(AGameStateBase* GameState)
{
    AMatchMenuGameState* MatchMenuGS = Cast<AMatchMenuGameState>(GameState);
    if (!MatchMenuGS)
    {
        return;
    }

    MatchMenuGS->OnSelectedMapChanged.AddDynamic(this, &ThisClass::HandleSelectedMapChanged);
    
    BeginSwitch(MatchMenuGS->GetSelectedLobbyMap(), true);
}

void AMatchMenuBackgroundSwitcher::HandleSelectedMapChanged(FGameplayTag NewMapTag)
{
    BeginSwitch(NewMapTag, false);
}

void AMatchMenuBackgroundSwitcher::BeginSwitch(const FGameplayTag NewMapTag, const bool bInstant)
{
    if (NewMapTag == CurrentTag || !BackgroundLevels.Contains(NewMapTag))
    {
        return;
    }

    // 전환 중에 또 바뀌면 마지막 요청만 기억해뒀다가 이어서 처리
    if (bSwitching)
    {
        PendingTag = NewMapTag;
        return;
    }

    bSwitching = true;
    TargetTag = NewMapTag;

    if (bInstant)
    {
        SwapLevels();
        return;
    }

    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        PC->PlayerCameraManager->StartCameraFade(0.f, 1.f, FadeDuration, FLinearColor::Black, false, true);
    }
    GetWorld()->GetTimerManager().SetTimer(TimerHandle_FadeOut, this, &ThisClass::SwapLevels, FadeDuration, false);
}

void AMatchMenuBackgroundSwitcher::SwapLevels()
{
    // 이전 배경 언로드
    if (const TSoftObjectPtr<UWorld>* OldLevel = BackgroundLevels.Find(CurrentTag))
    {
        FLatentActionInfo UnloadInfo;
        UnloadInfo.UUID = ++LatentUUID;
        UGameplayStatics::UnloadStreamLevelBySoftObjectPtr(this, *OldLevel, UnloadInfo, false);
    }

    // 새 배경 로드, 완료 시 HandleNewLevelLoaded 호출
    FLatentActionInfo LoadInfo;
    LoadInfo.CallbackTarget = this;
    LoadInfo.ExecutionFunction = FName("HandleNewLevelLoaded");
    LoadInfo.Linkage = 0;
    LoadInfo.UUID = ++LatentUUID;
    UGameplayStatics::LoadStreamLevelBySoftObjectPtr(this, BackgroundLevels[TargetTag], true, false, LoadInfo);
}

void AMatchMenuBackgroundSwitcher::HandleNewLevelLoaded()
{
    CurrentTag = TargetTag;
    bSwitching = false;

    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        PC->PlayerCameraManager->StartCameraFade(1.f, 0.f, FadeDuration, FLinearColor::Black, false, false);
    }

    // 전환 중에 들어온 요청 이어서 처리
    if (PendingTag.IsValid())
    {
        const FGameplayTag Next = PendingTag;
        PendingTag = FGameplayTag::EmptyTag;
        BeginSwitch(Next, false);
    }
}