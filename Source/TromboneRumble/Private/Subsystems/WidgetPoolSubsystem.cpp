// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/WidgetPoolSubsystem.h"

#include "Components/CanvasPanel.h"
#include "Components/PanelWidget.h"

void UWidgetPoolSubsystem::Prewarm(TSubclassOf<UUserWidget> InClass, int32 InCount, UUserWidget* InParent, UCanvasPanel* InCanvas, ESlateVisibility InVisible)
{
	if (!InClass || !InParent || !InCanvas) return;
	const FWidgetPoolKey Key{ InClass, InCanvas };
	FWidgetPool& Pool = Pools.FindOrAdd(Key);

	Pool.Class = InClass;

	for (int32 i = Pool.Inactive.Num(); i < InCount; ++i)
	{
		if (UUserWidget* CreatedWidget = SpawnPooledWidget(InClass, InParent, InCanvas))
		{
			CreatedWidget->SetVisibility(InVisible);
			Deactivate(CreatedWidget);
			Pool.Inactive.Add(CreatedWidget);
			Pool.TotalCreated++;
		}
	}
}

UUserWidget* UWidgetPoolSubsystem::Acquire(TSubclassOf<UUserWidget> InClass, UUserWidget* InParent, UCanvasPanel* InCanvas)
{
	if (!InClass || !InParent || !InCanvas) return nullptr;

	const FWidgetPoolKey Key{ InClass, InCanvas };
	FWidgetPool& Pool = Pools.FindOrAdd(Key);
	Pool.Class = InClass;

	UUserWidget* PooledWidget = nullptr;
	while (Pool.Inactive.Num() > 0 && !PooledWidget)
	{
		if (UUserWidget* Candidate = Pool.Inactive.Pop(EAllowShrinking::No))
		{
			PooledWidget = Candidate;
		}
	}
	if (!PooledWidget) PooledWidget = SpawnPooledWidget(InClass, InParent, InCanvas);
	if (!PooledWidget) return nullptr;

	Activate(PooledWidget);
	Pool.Active.Add(PooledWidget);

	return PooledWidget;
}

void UWidgetPoolSubsystem::Release(UUserWidget* InWidget)
{
	if (!IsValid(InWidget)) return;

	UCanvasPanel* Canvas = Cast<UCanvasPanel>(InWidget->GetParent());
	if (!Canvas)
	{
		Deactivate(InWidget);
		return;
	}

	const FWidgetPoolKey Key{ InWidget->GetClass(), Canvas };
	if (FWidgetPool* Pool = Pools.Find(Key))
	{
		if (Pool->Active.Remove(InWidget) > 0)
		{
			Deactivate(InWidget);
			Pool->Inactive.Add(InWidget);
		}
	}
	else
	{
		Deactivate(InWidget);
	}
}

UUserWidget* UWidgetPoolSubsystem::SpawnPooledWidget(TSubclassOf<UUserWidget> InClass, UUserWidget* InParent, UCanvasPanel* InCanvas)
{
	UUserWidget* CreatedWidget = CreateWidget<UUserWidget>(InParent, InClass);
	if (!CreatedWidget) return nullptr;
	if (InCanvas)
	{
		InCanvas->AddChild(CreatedWidget);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("NoCanvas"));
	}
	
	return CreatedWidget;
}

void UWidgetPoolSubsystem::Activate(UUserWidget* InWidget)
{
	if (!InWidget) return;
	//Visibility 설정을 하면 레이아웃 재계산을 해야하기 때문에 랙이 걸림.
	//InWidget->SetVisibility(ESlateVisibility::Visible);
	InWidget->SetIsEnabled(true);
	InWidget->SetRenderOpacity(1.f);
}

void UWidgetPoolSubsystem::Deactivate(UUserWidget* InWidget)
{
	if (!InWidget) return;
	//InWidget->SetVisibility(ESlateVisibility::Hidden);
	InWidget->SetRenderOpacity(0.f);
	InWidget->SetIsEnabled(false);
}
