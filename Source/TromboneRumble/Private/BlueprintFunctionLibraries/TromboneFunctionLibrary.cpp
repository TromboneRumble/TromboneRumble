// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintFunctionLibraries/TromboneFunctionLibrary.h"
#include "DeveloperSettings/GameMapDeveloperSettings.h"
#include "GameFramework/Actor.h"

FString UTromboneFunctionLibrary::GetMapPathByTag(FGameplayTag InMapTag)
{
    const UGameMapDeveloperSettings* Settings = GetDefault<UGameMapDeveloperSettings>();
    checkf(Settings, TEXT("GameMapDeveloperSettings is null"));

    const FSoftObjectPath* Found = Settings->GamePlayMap.Find(InMapTag);
    checkf(Found, TEXT("No map path for tag: %s"), *InMapTag.ToString());

    return Found->GetLongPackageName();

}

FName UTromboneFunctionLibrary::GetMapPackageNameByTag(FGameplayTag InMapTag)
{
    const FString Path = GetMapPathByTag(InMapTag);
    return FName(*Path);
}

void UTromboneFunctionLibrary::PrintDebug(const FString& Msg, int32 InKey, FLinearColor Color, float Duration,
	bool bRandomColor, bool bLog)
{
#if !UE_BUILD_SHIPPING
	FColor FinalColor = bRandomColor ? FColor::MakeRandomColor() : Color.ToFColor(true);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(InKey, Duration, FinalColor, Msg);
	}

	// 로그 출력
	if (bLog)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg);
	}
#endif
}