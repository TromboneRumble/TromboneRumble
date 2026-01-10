#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystem.h"

// Helper class for various methods to reduce the call hierarchy
struct FEasyOnlineHelper
{
public:
    FEasyOnlineHelper(const FString& InContextName, UWorld* InWorld, FName SystemName = NAME_None);
    
    bool IsValid() const;
    void GetUserID();
    
private:
    bool EnsureWorld() const;
    
public:
    FUniqueNetIdPtr UserID;
    IOnlineSubsystem* const OnlineSub;
    const FString& ContextName;
    TWeakObjectPtr<UWorld> WorldWeakPtr;
};