// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "UObject/Object.h"
#include "XRayBenchmark.generated.h"

class APawn;
class UXRayComponentBase;

/**
 * X-Ray 3종(실루엣/디더 페이드/원형 윈도우)의 프레임 비용을 순서대로 측정해 비교표를 낸다.
 *
 * 한 번에 한 방식만 켜지도록 기부착 컴포넌트를 전부 일시 정지시킨 뒤 페이즈마다 하나씩 살린다.
 * 붙어있지 않은 방식은 측정 동안만 임시로 만들었다가 페이즈가 끝나면 제거한다
 * (임시 컴포넌트의 머티리얼은 TromboneConfig의 Rendering|XRay 항목에서 온다).
 *
 * 측정 내내 벽에 가려진 자리에 서 있어야 의미가 있다 — 가려진 프레임 비율도 같이 출력된다.
 */
UCLASS()
class TROMBONERUMBLE_API UXRayBenchmark : public UObject
{
	GENERATED_BODY()

public:
	/** 측정 시작. 이미 실행 중이면 무시된다 */
	void Start(APawn* InPawn, float InSecondsPerPhase);

	bool IsRunning() const { return TickerHandle.IsValid(); }

	virtual void BeginDestroy() override;

private:
	struct FSample
	{
		TArray<float> FrameMs;
		double GameMs = 0.0;
		double RenderMs = 0.0;
		double GpuMs = 0.0;
		int32 OccludedFrames = 0;
		bool bSkipped = false;	// 해당 방식을 초기화하지 못해 측정을 건너뜀
	};

	bool Tick(float DeltaTime);
	void BeginPhase(int32 InPhaseIndex);
	void EndCurrentPhase();
	void Finish();
	void Report() const;
	static void Log(const FString& Message);

	static const TArray<TSubclassOf<UXRayComponentBase>>& GetPhaseClasses();
	static const TCHAR* GetPhaseLabel(int32 InPhaseIndex);

	TWeakObjectPtr<APawn> TargetPawn;

	// 측정 시작 시점에 이미 붙어있던 컴포넌트들. 전부 정지시켰다가 끝나면 원상복구한다
	UPROPERTY()
	TArray<TObjectPtr<UXRayComponentBase>> OriginalComponents;

	UPROPERTY()
	TObjectPtr<UXRayComponentBase> ActiveComponent;

	// ActiveComponent를 이번 측정을 위해 임시로 만들었는가 (그렇다면 페이즈 끝에 제거)
	bool bActiveIsTemporary = false;

	int32 PhaseIndex = INDEX_NONE;
	float SecondsPerPhase = 10.f;
	float Elapsed = 0.f;
	float Warmup = 0.f;

	TArray<FSample> Samples;
	FTSTicker::FDelegateHandle TickerHandle;
};
