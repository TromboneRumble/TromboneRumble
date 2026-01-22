#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEasySession, Log, All);

#define EASYSESSION_LOG_PREFIX *FString::Printf(TEXT("[%s] "), *FString(__FUNCTION__))

// UE_LOG_EASY(Log, TEXT("Message"));
// UE_LOG_EASY(Error, TEXT("Value: %d"), Value);
#define UE_LOG_EASY(Verbosity, Format, ...) \
{ \
    UE_LOG(LogEasySession, Verbosity, TEXT("%s%s"), EASYSESSION_LOG_PREFIX, *FString::Printf(Format, ##__VA_ARGS__)); \
}

#define UE_PRINT_EASY(Verbosity, Format, ...) \
{ \
    UE_LOG_EASY(Verbosity, Format, ##__VA_ARGS__); \
    if (GEngine) \
    { \
        FColor LogColor = FColor::Cyan; \
        if (ELogVerbosity::Verbosity == ELogVerbosity::Error) \
        { \
            LogColor = FColor::Red; \
        } \
        else if (ELogVerbosity::Verbosity == ELogVerbosity::Warning) \
        { \
            LogColor = FColor::Yellow; \
        } \
        FString FinalMsg = FString::Printf(TEXT("%s%s"), EASYSESSION_LOG_PREFIX, *FString::Printf(Format, ##__VA_ARGS__)); \
        GEngine->AddOnScreenDebugMessage((uint64)-1, 5.0f, LogColor, FinalMsg); \
    } \
}

// UE_CLOG_EASY(bFailed, Error, TEXT("Failed!"));
#define UE_CLOG_EASY(Conditional, Verbosity, Format, ...) \
{ \
    UE_CLOG(Conditional, LogEasySession, Verbosity, TEXT("%s%s"), EASYSESSION_LOG_PREFIX, *FString::Printf(Format, ##__VA_ARGS__)); \
}