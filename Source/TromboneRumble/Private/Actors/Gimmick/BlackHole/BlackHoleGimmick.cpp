// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Actors/Gimmick/BlackHole/BlackHoleGimmick.h"
#include "Components/SphereComponent.h"
#include "Data/Gimmick/BlackHoleGimmickConfig.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "Utilities/TromboneLogs.h"

#if !UE_BUILD_SHIPPING
TAutoConsoleVariable<int32> CVarBlackHoleDebug(
	TEXT("Trombone.BlackHole.Debug"),
	0,
	TEXT("1이면 블랙홀의 상태와 두 반경을 화면에 표시"));
#endif

ABlackHoleGimmick::ABlackHoleGimmick()
{
	bReplicates = true;
	bAlwaysRelevant = true;
	GimmickType = EGimmickType::BlackHole;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	InnerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InnerSphere"));
	SetRootComponent(InnerSphere);
	InnerSphere->SetSphereRadius(100.f);
	InnerSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InnerSphere->SetGenerateOverlapEvents(false);
	InnerSphere->SetCanEverAffectNavigation(false);
}

void ABlackHoleGimmick::BeginPlay()
{
	Super::BeginPlay();

	if (InnerSphere)
	{
		InnerSphere->SetSphereRadius(GetInnerRadius(), false);
	}

#if !UE_BUILD_SHIPPING
	IConsoleVariable* DebugVariable = CVarBlackHoleDebug.AsVariable();
	DebugVariable->OnChangedDelegate().AddUObject(this, &ThisClass::HandleDebugCVarChanged);
	HandleDebugCVarChanged(DebugVariable);
#endif
}

void ABlackHoleGimmick::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
#if !UE_BUILD_SHIPPING
	CVarBlackHoleDebug.AsVariable()->OnChangedDelegate().RemoveAll(this);
#endif

	// Deactivate 가 전부 지우지만, Deactivate 없이 파괴될 수도 있다
	GetWorldTimerManager().ClearTimer(ScheduleTimerHandle);
	GetWorldTimerManager().ClearTimer(PhaseTimerHandle);

	Super::EndPlay(EndPlayReason);
}

void ABlackHoleGimmick::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, State);
	DOREPLIFETIME(ThisClass, ActiveStartServerTime);
}

void ABlackHoleGimmick::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (InnerSphere)
	{
		// 콜리전이 꺼져 있고 매 프레임 자라므로 오버랩을 다시 계산할 이유가 없다
		InnerSphere->SetSphereRadius(GetInnerRadius(), false);
	}

	DebugDraw();
}

void ABlackHoleGimmick::Activate()
{
	const bool bWasActive = IsActive();

	Super::Activate();

	if (!bWasActive && HasAuthority())
	{
		ScheduleNext(GetConfig<UBlackHoleGimmickConfig>().Schedule.PickFirstDelay());
	}
}

void ABlackHoleGimmick::Deactivate()
{
	if (HasAuthority())
	{
		SetState(EBlackHoleState::Idle);
	}

	// Super clears the timers
	Super::Deactivate();
}

void ABlackHoleGimmick::ForceTrigger()
{
	if (!HasAuthority() || State != EBlackHoleState::Idle) return;

	GetWorldTimerManager().ClearTimer(ScheduleTimerHandle);
	StartWarning();
}

void ABlackHoleGimmick::ScheduleNext(const float Delay)
{
	if (!HasAuthority()) return;

	GetWorldTimerManager().ClearTimer(ScheduleTimerHandle);
	GetWorldTimerManager().SetTimer(ScheduleTimerHandle, this, &ThisClass::StartWarning, FMath::Max(0.01f, Delay), false);
}

void ABlackHoleGimmick::StartWarning()
{
	if (!HasAuthority()) return;

	SetState(EBlackHoleState::Warning);

	GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &ThisClass::StartActive,
		FMath::Max(0.01f, GetConfig<UBlackHoleGimmickConfig>().Schedule.WarningDuration), false);
}

void ABlackHoleGimmick::StartActive()
{
	if (!HasAuthority()) return;

	// 반경 계산의 기준 시각. 상태와 같은 번치로 가도록 SetState 보다 먼저 대입한다
	if (const AGameStateBase* GameState = GetWorld() ? GetWorld()->GetGameState() : nullptr)
	{
		ActiveStartServerTime = GameState->GetServerWorldTimeSeconds();
	}

	SetState(EBlackHoleState::Active);

	GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &ThisClass::StartCollapse,
		GetConfig<UBlackHoleGimmickConfig>().ActiveDuration, false);
}

void ABlackHoleGimmick::StartCollapse()
{
	if (!HasAuthority()) return;

	SetState(EBlackHoleState::Collapse);

	GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &ThisClass::EndCollapse,
		FMath::Max(0.05f, GetConfig<UBlackHoleGimmickConfig>().CollapseDuration), false);
}

void ABlackHoleGimmick::EndCollapse()
{
	if (!HasAuthority()) return;

	SetState(EBlackHoleState::Idle);

	ScheduleNext(GetConfig<UBlackHoleGimmickConfig>().Schedule.PickInterval());
}

void ABlackHoleGimmick::SetState(const EBlackHoleState NewState)
{
	if (!HasAuthority() || State == NewState) return;

	State = NewState;

	OnRep_State();  // 리슨 서버 호스트에서도 구독자 알림과 연출이 돌게 수동 호출
	ForceNetUpdate();
}

void ABlackHoleGimmick::OnRep_State()
{
	HandleStateChanged();
}

void ABlackHoleGimmick::HandleStateChanged()
{
	// 구독자에게 먼저 알린다. "상태가 바뀌었다"는 사실이지 렌더링 단계가 아니라서 데디 서버에서도 나가야 한다
	OnBlackHoleStateChangedDelegate.Broadcast(State);

	UpdateTickEnabled();

	UE_LOG(LogBlackHole, Log, TEXT("[%s] state -> %s"),
		HasAuthority() ? TEXT("Server") : TEXT("Client"),
		*UEnum::GetValueAsString(State));

	if (GetNetMode() == NM_DedicatedServer || !GetWorld() || GetWorld()->bIsTearingDown) return;

	OnBlackHoleStateChanged(State);
}

float ABlackHoleGimmick::GetGrowthAlpha() const
{
	if (State == EBlackHoleState::Collapse) return 1.f;
	if (State != EBlackHoleState::Active) return 0.f;

	const AGameStateBase* GameState = GetWorld() ? GetWorld()->GetGameState() : nullptr;
	if (!GameState) return 0.f;

	const float Duration = FMath::Max(0.01f, GetConfig<UBlackHoleGimmickConfig>().ActiveDuration);
	return FMath::Clamp((GameState->GetServerWorldTimeSeconds() - ActiveStartServerTime) / Duration, 0.f, 1.f);
}

float ABlackHoleGimmick::GetInfluenceRadius() const
{
	const UBlackHoleGimmickConfig& Config = GetConfig<UBlackHoleGimmickConfig>();
	return FMath::Lerp(Config.InfluenceRadiusStart, Config.InfluenceRadiusEnd, GetGrowthAlpha());
}

float ABlackHoleGimmick::GetInnerRadius() const
{
	const UBlackHoleGimmickConfig& Config = GetConfig<UBlackHoleGimmickConfig>();
	return FMath::Lerp(Config.InnerRadiusStart, Config.InnerRadiusEnd, GetGrowthAlpha());
}

bool ABlackHoleGimmick::IsInsideInfluence(const FVector& Location) const
{
	return IsPulling() && FVector::DistSquared(Location, GetActorLocation()) <= FMath::Square(GetInfluenceRadius());
}

bool ABlackHoleGimmick::IsInsideInner(const FVector& Location) const
{
	return IsPulling() && FVector::DistSquared(Location, GetActorLocation()) <= FMath::Square(GetInnerRadius());
}

void ABlackHoleGimmick::UpdateTickEnabled()
{
	bool bDebug = false;
#if !UE_BUILD_SHIPPING
	bDebug = CVarBlackHoleDebug.GetValueOnGameThread() != 0;
#endif

	SetActorTickEnabled(IsPulling() || bDebug);
}

void ABlackHoleGimmick::HandleDebugCVarChanged(IConsoleVariable* Variable)
{
	UpdateTickEnabled();
}

void ABlackHoleGimmick::DebugDraw() const
{
#if !UE_BUILD_SHIPPING
	if (CVarBlackHoleDebug.GetValueOnGameThread() == 0) return;

	const UWorld* World = GetWorld();
	if (!World) return;

	const FVector Center = GetActorLocation();
	DrawDebugSphere(World, Center, GetInfluenceRadius(), 24, FColor::Cyan, false, -1.f, 0, 2.f);
	DrawDebugSphere(World, Center, GetInnerRadius(), 16, FColor::Red, false, -1.f, 0, 2.f);

	if (!GEngine) return;

	const FTimerHandle& Handle = (State == EBlackHoleState::Idle) ? ScheduleTimerHandle : PhaseTimerHandle;
	const float Seconds = GetWorldTimerManager().GetTimerRemaining(Handle);
	const FString Remaining = Seconds >= 0.f ? FString::Printf(TEXT("%.1fs"), Seconds) : TEXT("타이머 없음");

	const FString Text = FString::Printf(TEXT("[블랙홀 %s] 상태 %s | 남은 시간 %s | 영향 반경 %.0f | 포획 반경 %.0f"),
		HasAuthority() ? TEXT("서버") : TEXT("클라"),
		*UEnum::GetDisplayValueAsText(State).ToString(),
		*Remaining,
		GetInfluenceRadius(),
		GetInnerRadius());
	GEngine->AddOnScreenDebugMessage(static_cast<uint64>(GetUniqueID()), 1.f, FColor::Purple, Text);
#endif
}
