// Fill out your copyright notice in the Description page of Project Settings.

#include "EasySessionUtils.h"
#include "OnlineSubsystemUtils.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "EasySessionLog.h"

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

	UGameInstance* GI = WorldWeakPtr->GetGameInstance();
	ULocalPlayer* LP = GI ? GI->GetFirstGamePlayer() : nullptr;
    
	if (!LP)
	{
		UE_LOG_EASY(Warning, TEXT("[%s] Failed to retrieve LocalPlayer."), *ContextName);
		return;
	}

	FUniqueNetIdRepl NetIdRepl = LP->GetPreferredUniqueNetId();
	if (!NetIdRepl.IsValid() || !NetIdRepl.GetUniqueNetId().IsValid())
	{
		UE_LOG_EASY(Warning, TEXT("[%s] Invalid UniqueNetId."), *ContextName);
		return;
	}

	UserID = NetIdRepl.GetUniqueNetId();
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