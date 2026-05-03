// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Subsystems/ToastSubsystem.h"
#include "CommonUserWidget.h"
#include "DeveloperSettings/TromboneConfig.h"
#include "UI/UserWidgets/Common/Toast/ToastItemWidget.h"
#include "UI/UserWidgets/Common/Toast/ToastContainerWidget.h"

UToastSubsystem::UToastSubsystem()
	: ToastPolicy(EToastSystemPolicy::Queue),
	  bIsToastShowing(false)
{
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
		ToastContainer->AddToViewport();
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

void UToastSubsystem::OnPostLoadMap(UWorld* NewWorld)
{
	ClearToastPool();
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
