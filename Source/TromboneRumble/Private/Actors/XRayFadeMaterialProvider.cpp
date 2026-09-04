// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Actors/XRayFadeMaterialProvider.h"
#include "Data/XRayFadeMaterialMap.h"
#include "Kismet/GameplayStatics.h"
#include "Utilities/DebugHelper.h"

AXRayFadeMaterialProvider::AXRayFadeMaterialProvider()
{
	PrimaryActorTick.bCanEverTick = false;
	// 순수 데이터 홀더라 복제할 것도, 클라이언트에서 스폰될 일도 없다
	bReplicates = false;
}

UXRayFadeMaterialMap* AXRayFadeMaterialProvider::FindMapInLevel(const UObject* WorldContextObject)
{
	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(WorldContextObject, StaticClass(), Found);

	if (Found.Num() == 0)
	{
		return nullptr;
	}

	if (Found.Num() > 1)
	{
		LOG_WITH_CURRENT_CONTEXT(Warning, FString::Printf(TEXT("Multiple XRayFadeMaterialProviders found (%d), using first"), Found.Num()));
	}

	const AXRayFadeMaterialProvider* Provider = Cast<AXRayFadeMaterialProvider>(Found[0]);
	return Provider ? Provider->FadeMaterialMap.Get() : nullptr;
}
