// Fill out your copyright notice in the Description page of Project Settings.

#include "EasySessionUtils.h"
#include "OnlineSubsystemUtils.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "EasySessionLog.h"

DEFINE_LOG_CATEGORY(LogEasySession);

FEasyOnlineHelper::FEasyOnlineHelper(const FString& InContextName, UWorld* InWorld, FName SystemName)
    : OnlineSub(Online::GetSubsystem(InWorld)),
    ContextName(InContextName),
    WorldWeakPtr(InWorld)
{
    if (OnlineSub == nullptr)
    {
        UE_LOG_EASY(Error, TEXT("[%s] Invalid or uninitialized OnlineSubsystem."), *ContextName);
    }
}

bool FEasyOnlineHelper::IsValid() const
{
    return UserID.IsValid() && (OnlineSub != nullptr);
}

void FEasyOnlineHelper::GetUserID()
{
    if (!EnsureWorld()) return;
        
    UserID.Reset();
        
    UGameInstance* GameInstance = WorldWeakPtr->GetGameInstance();
    if (!GameInstance)
    {
       UE_LOG_EASY(Warning, TEXT("[%s] GameInstance is invalid."), *ContextName);
       return;
    }

    ULocalPlayer* LocalPlayer = GameInstance->GetFirstGamePlayer();
    if (!LocalPlayer)
    {
       UE_LOG_EASY(Warning, TEXT("[%s] Could not find LocalPlayer."), *ContextName);
       return;
    }

    FUniqueNetIdRepl NetIdRepl = LocalPlayer->GetPreferredUniqueNetId();
    if (!NetIdRepl.IsValid())
    {
       UE_LOG_EASY(Warning, TEXT("[%s] LocalPlayer has no valid UniqueNetId."), *ContextName);
       return;
    }

    UserID = NetIdRepl.GetUniqueNetId();
    if (!UserID.IsValid())
    {
       UE_LOG_EASY(Warning, TEXT("[%s] Failed to get UniqueNetId from LocalPlayer."), *ContextName);
       return;
    }
}

bool FEasyOnlineHelper::EnsureWorld() const
{
    if (!WorldWeakPtr.IsValid())
    {
       UE_LOG_EASY(Error, TEXT("[%s] World is invalid."), *ContextName);
       return false;
    }
    return true;
}