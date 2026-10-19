// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Subsystems/TweenSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"

void UTweenSubsystem::DoTween(UUserWidget* InWidget, const FVector2D InTargetScale, const float InDuration, const ETweenCurveType InCurveType, const FSimpleDelegate& InCompletionDelegate)
{
	if (!InWidget)
	{
		return;
	}

	ActiveTweens.RemoveAll([InWidget](const FTweenRuntimeInfo& Info) {
		return Info.TargetWidget == InWidget;
	});

	FTweenRuntimeInfo NewTween;
	NewTween.TargetWidget = InWidget;
	NewTween.StartScale = InWidget->GetRenderTransform().Scale;
	NewTween.TargetScale = InTargetScale;
	NewTween.Duration = FMath::Max(InDuration, 0.001f); // !division by zero
	NewTween.CurveType = InCurveType;
	NewTween.ElapsedTime = 0.f;
	NewTween.CompletionCallback = InCompletionDelegate;

	ActiveTweens.Add(NewTween);
}

float UTweenSubsystem::EaseOutBack(float T)
{
	const float S = 1.70158f;
	T -= 1.f;
	return T * T * ((S + 1.f) * T + S) + 1.f;
}

float UTweenSubsystem::GetEasedAlpha(const float Alpha, const ETweenCurveType CurveType) const
{
	switch (CurveType)
	{
	case ETweenCurveType::Linear:
		return Alpha;
	case ETweenCurveType::EaseIn:
		return Alpha * Alpha;
	case ETweenCurveType::EaseOut:
		return 1.f - (1.f - Alpha) * (1.f - Alpha);
	case ETweenCurveType::EaseOutBack:
		return EaseOutBack(Alpha);
	default:
		return Alpha;
	}
}

void UTweenSubsystem::OnWorldCleanUp(UWorld* InWorld, bool bSessionEnded, bool bCleanupResources)
{
	// Only the tweens of widgets in the world being cleaned up stop, so a tween started for the next map or in another game instance keeps running.
	ActiveTweens.RemoveAll([InWorld](const FTweenRuntimeInfo& Tween)
	{
		return !Tween.TargetWidget.IsValid() || Tween.TargetWidget->GetWorld() == InWorld;
	});
}

void UTweenSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	FWorldDelegates::OnWorldCleanup.AddUObject(this, &ThisClass::OnWorldCleanUp);
}

void UTweenSubsystem::Deinitialize()
{
	FWorldDelegates::OnWorldCleanup.RemoveAll(this);
	ActiveTweens.Empty();
	
	Super::Deinitialize();
}

void UTweenSubsystem::Tick(const float DeltaTime)
{
	for (int32 i = ActiveTweens.Num() - 1; i >= 0; --i)
	{
		FTweenRuntimeInfo& Tween = ActiveTweens[i];

		if (!Tween.TargetWidget.IsValid())
		{
			ActiveTweens.RemoveAt(i);
			continue;
		}

		Tween.ElapsedTime += DeltaTime;
		const float Alpha = FMath::Clamp(Tween.ElapsedTime / Tween.Duration, 0.f, 1.f);
        
		float EasedAlpha = GetEasedAlpha(Alpha, Tween.CurveType);

		const FVector2D CurrentScale = FMath::Lerp(Tween.StartScale, Tween.TargetScale, EasedAlpha);
		Tween.TargetWidget->SetRenderScale(CurrentScale);

		if (Alpha >= 1.f)
		{
			FSimpleDelegate Callback = ActiveTweens[i].CompletionCallback;
			ActiveTweens.RemoveAt(i);
			Callback.ExecuteIfBound();
		}
	}
}
