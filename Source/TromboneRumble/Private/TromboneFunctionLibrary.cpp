// Fill out your copyright notice in the Description page of Project Settings.


#include "TromboneFunctionLibrary.h"
#include "DeveloperSettings/GameMapDeveloperSettings.h"

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
