// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Subsystems/ToastSubsystem.h"
#include "CommonUserWidget.h"
#include "EasySessionStatics.h"
#include "GameMapsSettings.h"
#include "TromboneGamePlayTags.h"
#include "DeveloperSettings/TromboneConfig.h"
#include "Framework/TromboneGameInstance.h"
#include "UI/UserWidgets/Common/Toast/ToastItemWidget.h"
#include "UI/UserWidgets/Common/Toast/ToastContainerWidget.h"

namespace
{
	/** @return Whether World is the transition map a seamless travel passes through. */
	bool IsTransitionMap(const UWorld* World)
	{
		const FString TransitionMap = FPackageName::GetShortName(GetDefault<UGameMapsSettings>()->TransitionMap.GetLongPackageName());
		return !TransitionMap.IsEmpty() && UWorld::RemovePIEPrefix(World->GetMapName()) == TransitionMap;
	}
}

UToastSubsystem::UToastSubsystem()
	: ToastPolicy(EToastSystemPolicy::Queue),
	  bIsToastShowing(false)
{
}

UToastSubsystem* UToastSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}
    
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (const UGameInstance* GI = World->GetGameInstance())
		{
			return GI->GetSubsystem<UToastSubsystem>();
		}
	}
	
	return nullptr;
}

void UToastSubsystem::ShowToast(const FToastRequest& InRequest)
{
	ToastQueue.Enqueue(InRequest);
	
	if (ToastPolicy == EToastSystemPolicy::Override && bIsToastShowing && ToastContainer)
	{
		if (UUserWidget* ToastWidget = ToastContainer->GetCurrentActiveToast())
		{
			if (UToastItemWidget* CurrentToast = Cast<UToastItemWidget>(ToastWidget))
			{
				CurrentToast->CloseToastImmediately(); 
				return;
			}
		}
	}

	if (!bIsToastShowing)
	{
		ProcessNextToast();
	}
}

UToastItemWidget* UToastSubsystem::GetOrCreateToastWidget()
{
	if (ToastPool.Num() > 0)
	{
		UToastItemWidget* Widget = ToastPool.Pop();
		if (IsValid(Widget)) 
		{
			return Widget;
		}
	}
	
	if (SimpleToastItemClass)
	{
		return CreateWidget<UToastItemWidget>(GetWorld(), SimpleToastItemClass);
	}
	
	UE_LOG(LogTemp, Error, TEXT("SimpleToastItemClass is not set in UToastSubsystem. Please check your UTromboneConfig settings."));
	return nullptr;
}

void UToastSubsystem::ProcessNextToastInternal(const FToastRequest& InRequest)
{
	bIsToastShowing = true;

	if (!ToastContainer && ToastContainerClass)
	{
		ToastContainer = CreateWidget<UToastContainerWidget>(GetWorld(), ToastContainerClass);
		ToastContainer->AddToViewport(10000);
	}

	if (UToastItemWidget* NewToast = GetOrCreateToastWidget())
	{
		ToastContainer->AddToastItem(NewToast, InRequest.Position);
		
		FSimpleDelegate OnToastFinishedCallback;
		OnToastFinishedCallback.BindUObject(this, &ThisClass::ProcessNextToast);
		
		NewToast->InitializeToast(InRequest, OnToastFinishedCallback);
	}
}

void UToastSubsystem::ProcessNextToast()
{
	FToastRequest NextRequest;
	if (ToastQueue.Dequeue(NextRequest))
	{
		ProcessNextToastInternal(NextRequest);
	}
	else
	{
		bIsToastShowing = false;
	}
}

void UToastSubsystem::ShowNotice(const FGameplayTag& Notice)
{
	if (const UTromboneGameInstance* GI = Cast<UTromboneGameInstance>(GetGameInstance()))
	{	
		ShowToast(FToastRequest(GI->GetCommonUIText(Notice.GetTagName().ToString())));
	}
}

void UToastSubsystem::ShowNoticeAfterTravel(const FGameplayTag& Notice)
{
	NoticesAfterTravel.AddUnique(Notice);
}

void UToastSubsystem::OnPostLoadMap(UWorld* NewWorld)
{
	ClearToastPool();

	// A seamless travel loads the transition map first, and the notices belong to the map after it
	if (!NewWorld || IsTransitionMap(NewWorld))
	{
		return;
	}

	// EasySession records why a session ended before it sends the player to the menu, and hands the record out once
	if (UEasySessionStatics::ConsumePendingEasyDisconnectInfo(this).Reason != EEasyDisconnectReason::None)
	{
		NoticesAfterTravel.AddUnique(TromboneGamePlayTags::Trombone_UI_Notice_ConnectionLost);
	}

	for (const FGameplayTag& Notice : NoticesAfterTravel)
	{
		ShowNotice(Notice);
	}
	NoticesAfterTravel.Reset();
}

void UToastSubsystem::ClearToastPool()
{
	if (ToastContainer)
	{
		ToastContainer->RemoveFromParent();
		ToastContainer = nullptr;
	}

	ToastPool.Empty();
    
	bIsToastShowing = false;
	ToastQueue.Empty();
}

void UToastSubsystem::ReturnToPool(UToastItemWidget* ToastWidget)
{
	if (ToastWidget && !ToastPool.Contains(ToastWidget))
	{
		ToastPool.Push(ToastWidget);
	}
}

void UToastSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::OnPostLoadMap);
	
	if (const UTromboneConfig* Config = UTromboneConfig::Get())
	{
		if (!Config->ToastContainerWidgetClass.IsNull())
		{
			ToastContainerClass = Config->ToastContainerWidgetClass.LoadSynchronous();
		}
		
		if (!Config->SimpleToastWidgetClass.IsNull())
		{
			SimpleToastItemClass = Config->SimpleToastWidgetClass.LoadSynchronous();
		}
		
		ToastPolicy = Config->ToastSystemPolicy;
	}
}

void UToastSubsystem::Deinitialize()
{
	ClearToastPool();
	
	Super::Deinitialize();
}
