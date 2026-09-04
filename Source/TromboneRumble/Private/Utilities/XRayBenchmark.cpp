// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Utilities/XRayBenchmark.h"
#include "Camera/CameraComponent.h"
#include "Components/ActorComponents/XRayComponentBase.h"
#include "Components/ActorComponents/XRayTranslucentFadeComponent.h"
#include "Components/ActorComponents/XRaySilhouetteComponent.h"
#include "Components/ActorComponents/XRayWindowComponent.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"
#include "RenderTimer.h"
#include "RHIGlobals.h"

namespace
{
	// 방식 전환 직후 몇 프레임은 셰이더 컴파일/리소스 생성으로 튀므로 표본에서 제외
	constexpr float WarmupSeconds = 1.f;
}

const TArray<TSubclassOf<UXRayComponentBase>>& UXRayBenchmark::GetPhaseClasses()
{
	static const TArray<TSubclassOf<UXRayComponentBase>> PhaseClasses = {
		UXRaySilhouetteComponent::StaticClass(),
		UXRayTranslucentFadeComponent::StaticClass(),
		UXRayWindowComponent::StaticClass(),
	};
	return PhaseClasses;
}

const TCHAR* UXRayBenchmark::GetPhaseLabel(int32 InPhaseIndex)
{
	switch (InPhaseIndex)
	{
	case 0:  return TEXT("A 실루엣      ");
	case 1:  return TEXT("B 디더 페이드 ");
	case 2:  return TEXT("C 원형 윈도우 ");
	default: return TEXT("?             ");
	}
}

void UXRayBenchmark::Start(APawn* InPawn, float InSecondsPerPhase)
{
	if (IsRunning())
	{
		Log(TEXT("이미 측정 중입니다."));
		return;
	}
	if (!InPawn || !InPawn->FindComponentByClass<UCameraComponent>())
	{
		Log(TEXT("카메라를 가진 폰이 없습니다 — 인게임에서 실행하세요."));
		return;
	}

	TargetPawn = InPawn;
	SecondsPerPhase = FMath::Max(1.f, InSecondsPerPhase);
	Samples.Reset();
	Samples.SetNum(GetPhaseClasses().Num());

	// 기부착 컴포넌트를 전부 정지 — 한 번에 한 방식만 켜야 비교가 성립한다
	OriginalComponents.Reset();
	TArray<UXRayComponentBase*> Existing;
	InPawn->GetComponents<UXRayComponentBase>(Existing);
	for (UXRayComponentBase* Component : Existing)
	{
		Component->SetTrackingPaused(true);
		OriginalComponents.Add(Component);
	}

	Log(FString::Printf(
		TEXT("X-Ray 3종 측정 시작 - 방식당 %.0f초. 끝날 때까지 벽에 가려진 자리에 그대로 서 있으세요."),
		SecondsPerPhase));

	if (GEngine && GEngine->GetMaxFPS() > 0.f)
	{
		Log(FString::Printf(
			TEXT("경고: 프레임 제한이 %.0f FPS로 걸려 있어 차이가 가려집니다. t.MaxFPS 0 / r.VSync 0 후 다시 측정하세요."),
			GEngine->GetMaxFPS()));
	}

	static const auto* VSyncCVar = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.VSync"));
	if (VSyncCVar && VSyncCVar->GetValueOnGameThread() != 0)
	{
		Log(TEXT("경고: VSync가 켜져 있어 차이가 가려집니다. r.VSync 0 후 다시 측정하세요."));
	}

	TickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UXRayBenchmark::Tick));

	BeginPhase(0);
}

void UXRayBenchmark::BeginPhase(int32 InPhaseIndex)
{
	PhaseIndex = InPhaseIndex;
	Elapsed = 0.f;
	Warmup = WarmupSeconds;

	APawn* Pawn = TargetPawn.Get();
	if (!Pawn)
	{
		Finish();
		return;
	}

	const TSubclassOf<UXRayComponentBase> PhaseClass = GetPhaseClasses()[PhaseIndex];

	// 이미 붙어있는 같은 방식이 있으면 그걸 되살려 쓴다 (BP에서 설정한 값을 그대로 측정하기 위함)
	ActiveComponent = nullptr;
	bActiveIsTemporary = false;
	for (UXRayComponentBase* Component : OriginalComponents)
	{
		if (Component && Component->GetClass() == PhaseClass)
		{
			Component->SetTrackingPaused(false);
			ActiveComponent = Component;
			break;
		}
	}

	if (!ActiveComponent)
	{
		// 등록하면 BeginPlay가 돌아 스스로 켜진다
		UXRayComponentBase* Temp = NewObject<UXRayComponentBase>(Pawn, PhaseClass);
		Temp->RegisterComponent();
		ActiveComponent = Temp;
		bActiveIsTemporary = true;
	}

	// 초기화에 실패한 방식(머티리얼 미지정 등)은 잠들어 있으므로 측정해봐야 의미 없는 값이 나온다
	if (!ActiveComponent->IsEffectActive())
	{
		Samples[PhaseIndex].bSkipped = true;
		Log(FString::Printf(TEXT("%s: 초기화 실패 — 건너뜀 (머티리얼 설정 확인)"), GetPhaseLabel(PhaseIndex)));
		EndCurrentPhase();

		if (PhaseIndex + 1 < GetPhaseClasses().Num())
		{
			BeginPhase(PhaseIndex + 1);
		}
		else
		{
			Report();
			Finish();
		}
		return;
	}

	Log(FString::Printf(TEXT("%s측정 중..."), GetPhaseLabel(PhaseIndex)));
}

void UXRayBenchmark::EndCurrentPhase()
{
	if (!ActiveComponent)
	{
		return;
	}

	if (bActiveIsTemporary)
	{
		ActiveComponent->DestroyComponent();
	}
	else
	{
		// 원래 붙어있던 건 다음 페이즈 동안 다시 재워둔다 (마지막에 Finish가 일괄 복구)
		ActiveComponent->SetTrackingPaused(true);
	}

	ActiveComponent = nullptr;
	bActiveIsTemporary = false;
}

bool UXRayBenchmark::Tick(float DeltaTime)
{
	// 폰이 죽거나 레벨이 바뀌면 측정을 중단한다
	if (!TargetPawn.IsValid() || !ActiveComponent)
	{
		Log(TEXT("측정 중단 — 대상이 사라졌습니다."));
		Finish();
		return false;
	}

	if (Warmup > 0.f)
	{
		Warmup -= DeltaTime;
		return true;
	}

	FSample& Sample = Samples[PhaseIndex];
	Sample.FrameMs.Add(DeltaTime * 1000.f);
	Sample.GameMs += FPlatformTime::ToMilliseconds(GGameThreadTime);
	Sample.RenderMs += FPlatformTime::ToMilliseconds(GRenderThreadTime);
	Sample.GpuMs += FPlatformTime::ToMilliseconds(GGPUFrameTime);
	if (ActiveComponent->IsAnyOccluding())
	{
		++Sample.OccludedFrames;
	}

	Elapsed += DeltaTime;
	if (Elapsed < SecondsPerPhase)
	{
		return true;
	}

	EndCurrentPhase();

	if (PhaseIndex + 1 < GetPhaseClasses().Num())
	{
		BeginPhase(PhaseIndex + 1);
		return IsRunning();
	}

	Report();
	Finish();
	return false;
}

void UXRayBenchmark::Report() const
{
	auto Percentile = [](TArray<float> Values, float Fraction) -> float
	{
		if (Values.Num() == 0)
		{
			return 0.f;
		}
		Values.Sort();
		const int32 Index = FMath::Clamp(FMath::FloorToInt(Values.Num() * Fraction), 0, Values.Num() - 1);
		return Values[Index];
	};

	Log(TEXT("===== X-Ray 방식별 결과 (낮을수록 좋음) ====="));

	TArray<float> AvgFrame;
	AvgFrame.Init(0.f, Samples.Num());

	for (int32 Phase = 0; Phase < Samples.Num(); ++Phase)
	{
		const FSample& Sample = Samples[Phase];
		if (Sample.bSkipped)
		{
			Log(FString::Printf(TEXT("%s: 건너뜀"), GetPhaseLabel(Phase)));
			continue;
		}

		const int32 Frames = Sample.FrameMs.Num();
		if (Frames == 0)
		{
			Log(FString::Printf(TEXT("%s: 표본 없음"), GetPhaseLabel(Phase)));
			continue;
		}

		float Total = 0.f;
		for (const float Ms : Sample.FrameMs)
		{
			Total += Ms;
		}
		AvgFrame[Phase] = Total / Frames;

		Log(FString::Printf(
			TEXT("%s: %.2f ms (%.0f FPS) | p95 %.2f ms | Game %.2f | Render %.2f | GPU %.2f | 가려진 프레임 %d%%"),
			GetPhaseLabel(Phase),
			AvgFrame[Phase],
			AvgFrame[Phase] > 0.f ? 1000.f / AvgFrame[Phase] : 0.f,
			Percentile(Sample.FrameMs, 0.95f),
			static_cast<float>(Sample.GameMs / Frames),
			static_cast<float>(Sample.RenderMs / Frames),
			static_cast<float>(Sample.GpuMs / Frames),
			FMath::RoundToInt(100.f * Sample.OccludedFrames / Frames)));

		// 가려지지 않은 채로 측정하면 모든 방식이 비용 0이라 비교가 무의미하다
		if (Sample.OccludedFrames < Frames / 2)
		{
			Log(TEXT("  ^ 주의: 측정 시간의 절반 이상을 가려지지 않은 상태로 보냈습니다. 결과를 신뢰하지 마세요."));
		}
	}

	// 가장 싼 방식을 기준으로 나머지의 추가 비용을 보여준다
	int32 BestPhase = INDEX_NONE;
	for (int32 Phase = 0; Phase < AvgFrame.Num(); ++Phase)
	{
		if (AvgFrame[Phase] > 0.f && (BestPhase == INDEX_NONE || AvgFrame[Phase] < AvgFrame[BestPhase]))
		{
			BestPhase = Phase;
		}
	}
	if (BestPhase != INDEX_NONE)
	{
		for (int32 Phase = 0; Phase < AvgFrame.Num(); ++Phase)
		{
			if (Phase == BestPhase || AvgFrame[Phase] <= 0.f)
			{
				continue;
			}
			const float Diff = AvgFrame[Phase] - AvgFrame[BestPhase];
			Log(FString::Printf(TEXT("%s: 최저 대비 %+.2f ms (%+.1f%%)"),
				GetPhaseLabel(Phase), Diff, 100.f * Diff / AvgFrame[BestPhase]));
		}
		Log(FString::Printf(TEXT("최저 비용: %s"), GetPhaseLabel(BestPhase)));
	}

	Log(TEXT("주의: 디더 페이드는 masked 전환된 머티리얼의 상시 비용이 여기 안 잡힙니다 (가려지지 않을 때도 계속 지불)."));
}

void UXRayBenchmark::Finish()
{
	EndCurrentPhase();

	// 측정 전 상태로 복구
	for (UXRayComponentBase* Component : OriginalComponents)
	{
		if (Component)
		{
			Component->SetTrackingPaused(false);
		}
	}
	OriginalComponents.Reset();

	if (TickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
		TickerHandle.Reset();
	}

	PhaseIndex = INDEX_NONE;
}

void UXRayBenchmark::BeginDestroy()
{
	if (TickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
		TickerHandle.Reset();
	}
	Super::BeginDestroy();
}

void UXRayBenchmark::Log(const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("[XRayBench] %s"), *Message);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 30.f, FColor::Yellow, Message);
	}
}
