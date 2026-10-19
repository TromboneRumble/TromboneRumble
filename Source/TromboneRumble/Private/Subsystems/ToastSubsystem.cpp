// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Subsystems/ToastSubsystem.h"
#include "CommonUserWidget.h"
#include "GameMapsSettings.h"
#include "DeveloperSettings/ToastSettings.h"
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

void UToastSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::OnPostLoadMap);
	
	const UToastSettings* Settings = GetDefault<UToastSettings>();
	if (!Settings->ContainerWidgetClass.IsNull())
	{
		ToastContainerClass = Settings->ContainerWidgetClass.LoadSynchronous();
	}
	
	if (!Settings->ItemWidgetClass.IsNull())
	{
		SimpleToastItemClass = Settings->ItemWidgetClass.LoadSynchronous();
	}
	
	ToastPolicy = Settings->Policy;
}

void UToastSubsystem::Deinitialize()
{
	ClearToastPool();
	
	Super::Deinitialize();
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

void UToastSubsystem::ShowToast(const UObject* WorldContextObject, const FGameplayTag Notice)
{
	if (UToastSubsystem* Toasts = Get(WorldContextObject))
	{
		Toasts->QueueToast(Notice);
	}
}

void UToastSubsystem::ShowToastAfterTravel(const UObject* WorldContextObject, const FGameplayTag& Notice)
{
	if (UToastSubsystem* Toasts = Get(WorldContextObject))
	{
		Toasts->ToastsAfterTravel.AddUnique(Notice);
	}
}

void UToastSubsystem::ReturnToPool(UToastItemWidget* ToastWidget)
{
	if (ToastWidget && !ToastPool.Contains(ToastWidget))
	{
		ToastPool.Push(ToastWidget);
	}
}

void UToastSubsystem::OnPostLoadMap(UWorld* NewWorld)
{
	// PostLoadMapWithWorld reaches every game instance in the process, as in PIE with several players
	if (NewWorld && NewWorld->GetGameInstance() != GetGameInstance())
	{
		return;
	}

	ClearToastPool();

	// A seamless travel loads the transition map first, and the notices belong to the map after it
	if (!NewWorld || IsTransitionMap(NewWorld))
	{
		return;
	}

	OnCollectToastsAfterTravel.Broadcast();

	for (const FGameplayTag& Notice : ToastsAfterTravel)
	{
		QueueToast(Notice);
	}
	ToastsAfterTravel.Reset();
}

void UToastSubsystem::QueueToast(const FGameplayTag& Notice)
{
	const UToastSettings* Settings = GetDefault<UToastSettings>();
	if (Settings->NoticeStringTable.IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("Notice String Table is not set in Toast Settings, so notice %s is not shown."), *Notice.ToString());
		return;
	}

	const FName TableId(*Settings->NoticeStringTable.ToSoftObjectPath().GetAssetPathString());
	const FText Message = FText::FromStringTable(TableId, Notice.GetTagName().ToString());
	ToastQueue.Enqueue(FToastRequest(Message, Settings->Position, Settings->DisplayDuration));
	
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
	
	UE_LOG(LogTemp, Error, TEXT("Item Widget Class is not set. Please check Toast Settings in Project Settings."));
	return nullptr;
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
