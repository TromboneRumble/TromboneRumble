// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TweenSubsystem.generated.h"

UENUM(BlueprintType)
enum class ETweenCurveType : uint8
{
	// curve graph : https://ruyagames.tistory.com/24
	Linear,
	EaseIn,
	EaseOut,
	EaseOutBack,
};

USTRUCT()
struct FTweenRuntimeInfo
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<UUserWidget> TargetWidget;

	float ElapsedTime = 0.f;
	float Duration = 0.f;
	FVector2D StartScale = FVector2D::UnitVector;
	FVector2D TargetScale = FVector2D::UnitVector;
	ETweenCurveType CurveType = ETweenCurveType::Linear;
	FSimpleDelegate CompletionCallback;
};

/**
 * Handles tweens for UI. only supports scale
 */
UCLASS()
class TROMBONERUMBLE_API UTweenSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()
	
public:

	void DoTween(UUserWidget* InWidget, FVector2D InTargetScale, float InDuration = 1.0f, ETweenCurveType InCurveType = ETweenCurveType::Linear, const FSimpleDelegate& InCompletionDelegate = FSimpleDelegate());

	static float EaseOutBack(float T);
	
private:
	
	float GetEasedAlpha(const float Alpha, const ETweenCurveType CurveType) const;
	
	/** Clear tweens when world is changed */
	void OnWorldCleanUp(UWorld* InWorld);

private:

	UPROPERTY()
	TArray<FTweenRuntimeInfo> ActiveTweens;

public:
	
	// ~ Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// ~ End USubsystem Interface

	// ~ Begin FTickableGameObject Interface
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return ActiveTweens.Num() > 0; }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UTweenSubsystem, STATGROUP_Tickables); }
	// ~ End FTickableGameObject Interface
	
};
