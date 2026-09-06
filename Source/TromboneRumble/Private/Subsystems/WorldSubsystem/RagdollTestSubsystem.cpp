// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Subsystems/WorldSubsystem/RagdollTestSubsystem.h"
#include "Characters/TromboneCharacterBase.h"
#include "Components/ActorComponents/LobbyDirectorComponent.h"
#include "Components/ActorComponents/TromboneRagdollComponent.h"
#include "DeveloperSettings/RagdollTestSettings.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "Misc/App.h"
#include "Misc/FileHelper.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "UObject/UnrealType.h"
#include "Utilities/Defines.h"

#if WITH_EDITOR
static TAutoConsoleVariable<int32> CVarRagdollTest(
	TEXT("Trombone.Ragdoll.Test"),
	0,
	TEXT("0 off, 1 overlay, 2 overlay + CSV in Saved/RagdollTest. Client side, PIE only"));

static TAutoConsoleVariable<FString> CVarRagdollTestLabel(
	TEXT("Trombone.Ragdoll.Test.Label"),
	TEXT("manual"),
	TEXT("Tag written into every summary row. A campaign sets it per case"));

namespace
{
	/** Error the client has to get under after the server body stops, for the settle time. */
	constexpr float SettleErrorCm = 5.0f;

	/** Same value as the default RestSpeedThreshold of the ragdoll component. */
	constexpr float ServerRestSpeed = 20.0f;

	/** The run starts counting once the client pelvis first gets this close. Cuts off the catch-up after the drop teleport. */
	constexpr float SyncErrorCm = 100.0f;

	constexpr float GroundProbeRadius = 20.0f;
	constexpr float GroundProbeDistance = 30.0f;

	constexpr int32 OverlayKeyBase = 20000;
	constexpr int32 CampaignKey = 19999;

	const TCHAR* SummaryHeader = TEXT("Stamp,Label,Scenario,Player,Self,Samples,Duration,SyncTime,PelvisMean,PelvisP95,PelvisMax,PelvisAirMean,PelvisGroundMean,RotMean,RotMax,BodyMean,BodyMax,SettleTime,SnapCount,RecoverCount,FpsMean,PingMean,AgeMeanMs,TrueAgeMeanMs,FirstStateMs,Features,Overrides\n");

	float Percentile(TArray<float> Values, const float P)
	{
		if (Values.Num() == 0)
		{
			return 0.0f;
		}

		Values.Sort();
		const int32 Index = FMath::Clamp(FMath::RoundToInt(P * (Values.Num() - 1)), 0, Values.Num() - 1);
		return Values[Index];
	}

	float Median(TArray<float> Values)
	{
		if (Values.Num() == 0)
		{
			return 0.0f;
		}

		Values.Sort();
		const int32 Mid = Values.Num() / 2;
		return Values.Num() % 2 == 0 ? (Values[Mid - 1] + Values[Mid]) * 0.5f : Values[Mid];
	}

	FString TestDir()
	{
		return FPaths::ProjectSavedDir() / TEXT("RagdollTest");
	}

	void SetConsoleVariable(const TCHAR* Name, const int32 Value)
	{
		if (IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(Name))
		{
			// Console priority, so a value the user typed earlier does not win over the campaign
			Variable->Set(Value, ECVF_SetByConsole);
		}
	}
}
#endif

// ---------------------------------------------------------------- lifecycle

bool URagdollTestSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
#if WITH_EDITOR
	const UWorld* World = Cast<UWorld>(Outer);
	return Super::ShouldCreateSubsystem(Outer) && World && World->WorldType == EWorldType::PIE;
#else
	return false;
#endif
}

void URagdollTestSubsystem::Deinitialize()
{
#if WITH_EDITOR
	if (bCampaignActive)
	{
		EndCampaign(TEXT("world ended"));
	}

	for (const TPair<TWeakObjectPtr<ATromboneCharacterBase>, FRagdollTestRun>& Pair : Runs)
	{
		EndRun(Pair.Value);
	}
	Runs.Empty();
#endif

	Super::Deinitialize();
}

TStatId URagdollTestSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(URagdollTestSubsystem, STATGROUP_Tickables);
}

int32 URagdollTestSubsystem::GetMode() const
{
#if WITH_EDITOR
	return bCampaignActive ? 2 : CVarRagdollTest.GetValueOnGameThread();
#else
	return 0;
#endif
}

void URagdollTestSubsystem::Tick(const float DeltaTime)
{
#if WITH_EDITOR
	UWorld* World = GetWorld();
	if (GetMode() == 0 || !World || World->GetNetMode() != NM_Client)
	{
		Runs.Empty();
		return;
	}

	UWorld* Server = FindServerWorld();
	if (!Server)
	{
		return;
	}

	// Ticking after the world, so the ragdoll component has already moved the body this frame
	TSet<TWeakObjectPtr<ATromboneCharacterBase>> Active;
	int32 Index = 0;
	for (TActorIterator<ATromboneCharacterBase> It(World); It; ++It)
	{
		const ATromboneCharacterBase* Character = *It;
		const UTromboneRagdollComponent* Ragdoll = Character->GetRagdollComponent();
		if (!Ragdoll || !Ragdoll->IsRagdoll())
		{
			continue;
		}

		Active.Add(*It);
		FRagdollTestRun& Run = Runs.FindOrAdd(*It);
		if (!Run.bStarted)
		{
			BeginRun(Run, Character, Server);
		}

		Sample(Run, Character, DeltaTime);
		DrawOverlay(Run, Character, Index++, DeltaTime);
	}

	// A run ends when its ragdoll does
	for (auto It = Runs.CreateIterator(); It; ++It)
	{
		if (!Active.Contains(It.Key()))
		{
			EndRun(It.Value());
			It.RemoveCurrent();
		}
	}

	if (bCampaignActive)
	{
		TickCampaign();
	}
#endif
}

UWorld* URagdollTestSubsystem::FindServerWorld() const
{
#if WITH_EDITOR
	if (ServerWorld.IsValid())
	{
		return ServerWorld.Get();
	}

	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		UWorld* World = Context.World();
		if (Context.WorldType == EWorldType::PIE && World && World->GetNetMode() != NM_Client && World->GetNetMode() != NM_Standalone)
		{
			ServerWorld = World;
			return World;
		}
	}
#endif

	return nullptr;
}

URagdollTestSubsystem* URagdollTestSubsystem::FindClientInstance()
{
#if WITH_EDITOR
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		UWorld* World = Context.World();
		if (Context.WorldType == EWorldType::PIE && World && World->GetNetMode() == NM_Client)
		{
			return World->GetSubsystem<URagdollTestSubsystem>();
		}
	}
#endif

	return nullptr;
}

// ---------------------------------------------------------------- one run

void URagdollTestSubsystem::BeginRun(FRagdollTestRun& Run, const ATromboneCharacterBase* Character, UWorld* Server) const
{
#if WITH_EDITOR
	Run.bStarted = true;
	Run.StartTime = GetWorld()->GetTimeSeconds();
	Run.bLocallyControlled = Character->IsLocallyControlled();

	const APlayerState* PlayerState = Character->GetPlayerState();
	Run.PlayerName = PlayerState ? PlayerState->GetPlayerName() : Character->GetName();

	// Same player id on both worlds, actor names are not
	const int32 PlayerId = PlayerState ? PlayerState->GetPlayerId() : INDEX_NONE;
	for (TActorIterator<ATromboneCharacterBase> It(Server); It; ++It)
	{
		const APlayerState* ServerPlayerState = It->GetPlayerState();
		if (ServerPlayerState && ServerPlayerState->GetPlayerId() == PlayerId)
		{
			Run.ServerCharacter = *It;
			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[RagdollTest] No server character for %s"), *Run.PlayerName);
#endif
}

void URagdollTestSubsystem::Sample(FRagdollTestRun& Run, const ATromboneCharacterBase* Character, const float DeltaTime) const
{
#if WITH_EDITOR
	const ATromboneCharacterBase* ServerCharacter = Run.ServerCharacter.Get();
	const UTromboneRagdollComponent* Ragdoll = Character->GetRagdollComponent();
	const USkeletalMeshComponent* ClientMesh = Character->GetMesh();
	USkeletalMeshComponent* ServerMesh = ServerCharacter ? ServerCharacter->GetMesh() : nullptr;
	if (!Ragdoll || !ClientMesh || !ServerMesh)
	{
		return;
	}

	const FBodyInstance* ClientPelvis = ClientMesh->GetBodyInstance(TromboneBones::Pelvis);
	const FBodyInstance* ServerPelvis = ServerMesh->GetBodyInstance(TromboneBones::Pelvis);
	if (!ClientPelvis || !ServerPelvis)
	{
		return;
	}

	const UWorld* World = GetWorld();
	const FRagdollSyncStats& Stats = Ragdoll->GetSyncStats();

	// Before the first state the component is idle and its stats are from the previous ragdoll
	if (!Stats.bHasState)
	{
		return;
	}
	if (Run.FirstStateTime < 0.0f)
	{
		Run.FirstStateTime = World->GetTimeSeconds() - Run.StartTime;
	}

	const FTransform ClientTransform = ClientPelvis->GetUnrealWorldTransform();
	const FTransform ServerTransform = ServerPelvis->GetUnrealWorldTransform();

	FRagdollTestSample S;
	S.Time = World->GetTimeSeconds() - Run.StartTime;
	S.Fps = DeltaTime > 0.0f ? 1.0f / DeltaTime : 0.0f;

	const UPhysicsSettings* Physics = UPhysicsSettings::Get();
	S.PhysicsDt = FMath::Min(DeltaTime, Physics->bSubstepping ? Physics->MaxSubstepDeltaTime : Physics->MaxPhysicsDeltaTime);

	if (const APlayerController* LocalPC = World->GetFirstPlayerController())
	{
		S.PingMs = LocalPC->PlayerState ? LocalPC->PlayerState->GetPingInMilliseconds() : 0.0f;
	}

	S.bAirborne = IsPelvisAirborne(Character);
	S.PelvisError = FVector::Dist(ClientTransform.GetLocation(), ServerTransform.GetLocation());
	S.PelvisRotError = FMath::RadiansToDegrees(ClientTransform.GetRotation().AngularDistance(ServerTransform.GetRotation()));

	// Both meshes share one physics asset, so body indices line up
	float BodySum = 0.0f;
	int32 BodyCount = 0;
	const int32 PairCount = FMath::Min(ClientMesh->Bodies.Num(), ServerMesh->Bodies.Num());
	for (int32 i = 0; i < PairCount; ++i)
	{
		const FBodyInstance* ClientBody = ClientMesh->Bodies[i];
		const FBodyInstance* ServerBody = ServerMesh->Bodies[i];
		if (!ClientBody || !ServerBody || !ClientBody->IsValidBodyInstance() || !ServerBody->IsValidBodyInstance())
		{
			continue;
		}

		BodySum += FVector::Dist(ClientBody->GetUnrealWorldTransform().GetLocation(), ServerBody->GetUnrealWorldTransform().GetLocation());
		++BodyCount;
	}
	S.BodyError = BodyCount > 0 ? BodySum / BodyCount : 0.0f;

	S.ServerSpeed = ServerMesh->GetPhysicsLinearVelocity(TromboneBones::Pelvis).Size();
	S.TargetError = FVector::Dist(ClientTransform.GetLocation(), Stats.TargetPelvisLocation);
	S.PacketAge = Stats.PacketAge;
	S.CorrectionSpeed = Stats.CorrectionSpeed;
	if (const AGameStateBase* ServerGameState = ServerCharacter->GetWorld()->GetGameState())
	{
		S.TrueAge = ServerGameState->GetServerWorldTimeSeconds() - Stats.StateTimestamp;
	}

	Run.SnapCount = Stats.SnapCount;
	Run.PenetrationRecoveryCount = Stats.PenetrationRecoveryCount;
	Run.MaxPelvisError = FMath::Max(Run.MaxPelvisError, S.PelvisError);
	Run.Samples.Add(S);
#endif
}

void URagdollTestSubsystem::DrawOverlay(const FRagdollTestRun& Run, const ATromboneCharacterBase* Character, const int32 Index, const float DeltaTime) const
{
#if WITH_EDITOR
	const ATromboneCharacterBase* ServerCharacter = Run.ServerCharacter.Get();
	if (!ServerCharacter || Run.Samples.Num() == 0 || !GEngine)
	{
		return;
	}

	const FBodyInstance* ClientPelvis = Character->GetMesh()->GetBodyInstance(TromboneBones::Pelvis);
	const FBodyInstance* ServerPelvis = ServerCharacter->GetMesh()->GetBodyInstance(TromboneBones::Pelvis);
	if (!ClientPelvis || !ServerPelvis)
	{
		return;
	}

	const UWorld* World = GetWorld();
	const FTransform ClientTransform = ClientPelvis->GetUnrealWorldTransform();
	const FTransform ServerTransform = ServerPelvis->GetUnrealWorldTransform();
	const FVector ClientLoc = ClientTransform.GetLocation();
	const FVector ServerLoc = ServerTransform.GetLocation();
	const FRagdollSyncStats& Stats = Character->GetRagdollComponent()->GetSyncStats();

	// Green = client, Red = the target the client steers to, Yellow = server
	DrawDebugSphere(World, ClientLoc, 10.0f, 8, FColor::Green, false, -1.0f, 0, 1.0f);
	DrawDebugSphere(World, Stats.TargetPelvisLocation, 10.0f, 8, FColor::Red, false, -1.0f, 0, 1.0f);
	DrawDebugSphere(World, ServerLoc, 10.0f, 8, FColor::Yellow, false, -1.0f, 0, 1.0f);
	DrawDebugLine(World, ClientLoc, ServerLoc, FColor::Yellow, false, -1.0f, 0, 1.5f);

	constexpr float AxisLength = 40.0f;
	DrawDebugLine(World, ClientLoc, ClientLoc + ClientTransform.GetRotation().GetForwardVector() * AxisLength, FColor::Green, false, -1.0f, 0, 1.5f);
	DrawDebugLine(World, ClientLoc, ClientLoc + ClientTransform.GetRotation().GetUpVector() * AxisLength, FColor::Emerald, false, -1.0f, 0, 1.5f);
	DrawDebugLine(World, ServerLoc, ServerLoc + ServerTransform.GetRotation().GetForwardVector() * AxisLength, FColor::Yellow, false, -1.0f, 0, 1.5f);
	DrawDebugLine(World, ServerLoc, ServerLoc + ServerTransform.GetRotation().GetUpVector() * AxisLength, FColor::Orange, false, -1.0f, 0, 1.5f);

	const FRagdollTestSample& S = Run.Samples.Last();
	const FString Text = FString::Printf(
		TEXT("[%s %s] pelvis %.1f (max %.1f) | body %.1f | rot %.0f deg | snap %d | %s"),
		*Run.PlayerName, Run.bLocallyControlled ? TEXT("self") : TEXT("other"),
		S.PelvisError, Run.MaxPelvisError, S.BodyError, S.PelvisRotError,
		Run.SnapCount, S.bAirborne ? TEXT("air") : TEXT("ground"));
	GEngine->AddOnScreenDebugMessage(OverlayKeyBase + Index, DeltaTime, FColor::Cyan, Text);
#endif
}

void URagdollTestSubsystem::EndRun(const FRagdollTestRun& Run)
{
#if WITH_EDITOR
	if (Run.Samples.Num() == 0)
	{
		return;
	}

	FRagdollTestSummary S = Summarize(Run);
	const bool bWarmup = bCampaignActive && Steps.IsValidIndex(StepIndex) && !Steps[StepIndex].bRecord;
	if (bWarmup)
	{
		S.Label += TEXT(" (warm-up)");
	}
	PrintSummary(S);

	if (bWarmup || GetMode() < 2)
	{
		return;
	}

	const FString Stamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
	WriteRunCsv(Run, S, Stamp);
	AppendSummaryRow(S, Stamp);

	if (bCampaignActive)
	{
		CaseSummaries.Add(S);
	}
#endif
}

FRagdollTestSummary URagdollTestSubsystem::Summarize(const FRagdollTestRun& Run) const
{
	FRagdollTestSummary S;
#if WITH_EDITOR
	const int32 Num = Run.Samples.Num();

	// The server teleports to the drop point first, so the first frames are a catch-up, not sync quality
	int32 SyncIndex = 0;
	while (SyncIndex < Num && Run.Samples[SyncIndex].PelvisError > SyncErrorCm)
	{
		++SyncIndex;
	}
	const bool bSynced = SyncIndex < Num;
	if (!bSynced)
	{
		SyncIndex = 0;
	}
	const int32 Counted = Num - SyncIndex;

	TArray<float> PelvisErrors;
	PelvisErrors.Reserve(Counted);
	float PelvisSum = 0.0f, AirSum = 0.0f, GroundSum = 0.0f, RotSum = 0.0f, BodySum = 0.0f;
	float FpsSum = 0.0f, PingSum = 0.0f, AgeSum = 0.0f, TrueAgeSum = 0.0f;
	int32 AirCount = 0, GroundCount = 0;
	for (int32 i = SyncIndex; i < Num; ++i)
	{
		const FRagdollTestSample& Frame = Run.Samples[i];
		PelvisErrors.Add(Frame.PelvisError);
		PelvisSum += Frame.PelvisError;
		S.PelvisMax = FMath::Max(S.PelvisMax, Frame.PelvisError);
		if (Frame.bAirborne)
		{
			AirSum += Frame.PelvisError;
			++AirCount;
		}
		else
		{
			GroundSum += Frame.PelvisError;
			++GroundCount;
		}
		RotSum += Frame.PelvisRotError;
		S.RotMax = FMath::Max(S.RotMax, Frame.PelvisRotError);
		BodySum += Frame.BodyError;
		S.BodyMax = FMath::Max(S.BodyMax, Frame.BodyError);
		FpsSum += Frame.Fps;
		PingSum += Frame.PingMs;
		AgeSum += Frame.PacketAge;
		TrueAgeSum += Frame.TrueAge;
	}

	// Settle time: from the moment the server body stops for good, until the client error is small
	int32 RestStart = SyncIndex;
	for (int32 i = Num - 1; i >= SyncIndex; --i)
	{
		if (Run.Samples[i].ServerSpeed >= ServerRestSpeed)
		{
			RestStart = i + 1;
			break;
		}
	}
	for (int32 i = RestStart; i < Num; ++i)
	{
		if (Run.Samples[i].PelvisError <= SettleErrorCm)
		{
			S.SettleTime = Run.Samples[i].Time - Run.Samples[RestStart].Time;
			break;
		}
	}

	S.Label = CVarRagdollTestLabel.GetValueOnGameThread();
	S.Scenario = bCampaignActive && Steps.IsValidIndex(StepIndex) ? ScenarioName(Steps[StepIndex].Scenario) : TEXT("manual");
	if (bCampaignActive && CampaignCases.IsValidIndex(CaseIndex))
	{
		for (const TPair<FName, float>& Pair : CampaignCases[CaseIndex].Overrides)
		{
			S.Overrides += FString::Printf(TEXT("%s%s=%g"), S.Overrides.IsEmpty() ? TEXT("") : TEXT(";"), *Pair.Key.ToString(), Pair.Value);
		}
	}
	S.PlayerName = Run.PlayerName;
	S.bLocallyControlled = Run.bLocallyControlled;
	S.Features = UTromboneRagdollComponent::GetSyncFeatures();
	S.Samples = Counted;
	S.Duration = Run.Samples.Last().Time;
	S.SyncTime = bSynced ? Run.Samples[SyncIndex].Time : -1.0f;
	S.FirstStateMs = Run.FirstStateTime * 1000.0f;
	S.PelvisMean = PelvisSum / Counted;
	S.PelvisP95 = Percentile(PelvisErrors, 0.95f);
	S.AirMean = AirCount > 0 ? AirSum / AirCount : 0.0f;
	S.GroundMean = GroundCount > 0 ? GroundSum / GroundCount : 0.0f;
	S.RotMean = RotSum / Counted;
	S.BodyMean = BodySum / Counted;
	S.SnapCount = Run.SnapCount;
	S.RecoverCount = Run.PenetrationRecoveryCount;
	S.FpsMean = FpsSum / Counted;
	S.PingMean = PingSum / Counted;
	S.AgeMeanMs = AgeSum / Counted * 1000.0f;
	S.TrueAgeMeanMs = TrueAgeSum / Counted * 1000.0f;
#endif
	return S;
}

void URagdollTestSubsystem::PrintSummary(const FRagdollTestSummary& S) const
{
#if WITH_EDITOR
	TArray<FString> Lines;
	Lines.Add(FString::Printf(TEXT("[RagdollTest] %s %s | %s (%s) | features %d | %.1fs, %d samples, %.0f fps, ping %.0f ms"),
		*S.Label, *S.Scenario, *S.PlayerName, S.bLocallyControlled ? TEXT("self") : TEXT("other"), S.Features, S.Duration, S.Samples, S.FpsMean, S.PingMean));
	Lines.Add(FString::Printf(TEXT("  pelvis     mean %6.1f   p95 %6.1f   max %6.1f cm   (air %.1f / ground %.1f)"), S.PelvisMean, S.PelvisP95, S.PelvisMax, S.AirMean, S.GroundMean));
	Lines.Add(FString::Printf(TEXT("  rotation   mean %6.1f   max %6.1f deg"), S.RotMean, S.RotMax));
	Lines.Add(FString::Printf(TEXT("  body       mean %6.1f   max %6.1f cm"), S.BodyMean, S.BodyMax));
	Lines.Add(FString::Printf(TEXT("  sync       first %.2fs   settle %.2fs   snap %d   recover %d"), S.SyncTime, S.SettleTime, S.SnapCount, S.RecoverCount));
	Lines.Add(FString::Printf(TEXT("  age        used %.1f ms   true %.1f ms   first state after %.0f ms"), S.AgeMeanMs, S.TrueAgeMeanMs, S.FirstStateMs));
	for (const FString& Line : Lines)
	{
		UE_LOG(LogTemp, Log, TEXT("%s"), *Line);
	}
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 12.0f, FColor::Yellow, FString::Join(Lines, TEXT("\n")));
	}
#endif
}

void URagdollTestSubsystem::WriteRunCsv(const FRagdollTestRun& Run, const FRagdollTestSummary& S, const FString& Stamp) const
{
#if WITH_EDITOR
	const FString FileName = FString::Printf(TEXT("%s_%s_%s_%s%s.csv"), *Stamp, *S.Label, *S.Scenario, *FPaths::MakeValidFileName(Run.PlayerName), Run.bLocallyControlled ? TEXT("_self") : TEXT(""));

	// One row per frame
	FString Rows = TEXT("Time,Fps,PhysicsDt,PingMs,Airborne,PelvisError,PelvisRotError,BodyError,ServerSpeed,TargetError,PacketAge,TrueAge,CorrectionSpeed\n");
	for (const FRagdollTestSample& Frame : Run.Samples)
	{
		Rows += FString::Printf(TEXT("%.4f,%.1f,%.4f,%.0f,%d,%.2f,%.2f,%.2f,%.1f,%.2f,%.4f,%.4f,%.1f\n"),
			Frame.Time, Frame.Fps, Frame.PhysicsDt, Frame.PingMs, Frame.bAirborne ? 1 : 0, Frame.PelvisError, Frame.PelvisRotError, Frame.BodyError,
			Frame.ServerSpeed, Frame.TargetError, Frame.PacketAge, Frame.TrueAge, Frame.CorrectionSpeed);
	}
	FFileHelper::SaveStringToFile(Rows, *(TestDir() / FileName));
	UE_LOG(LogTemp, Log, TEXT("[RagdollTest] Wrote %s"), *FileName);
#endif
}

void URagdollTestSubsystem::AppendSummaryRow(const FRagdollTestSummary& S, const FString& Stamp) const
{
#if WITH_EDITOR
	// One row per run, appended so ten drops of one setup sit next to each other
	const FString SummaryPath = TestDir() / TEXT("summary.csv");
	FString Row;
	if (!IFileManager::Get().FileExists(*SummaryPath))
	{
		Row += SummaryHeader;
	}
	Row += FString::Printf(TEXT("%s,%s,%s,%s,%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d,%.0f,%.0f,%.1f,%.1f,%.0f,%d\n"),
		*Stamp, *S.Label, *S.Scenario, *S.PlayerName, S.bLocallyControlled ? 1 : 0, S.Samples, S.Duration, S.SyncTime, S.PelvisMean, S.PelvisP95, S.PelvisMax,
		S.AirMean, S.GroundMean, S.RotMean, S.RotMax, S.BodyMean, S.BodyMax, S.SettleTime,
		S.SnapCount, S.RecoverCount, S.FpsMean, S.PingMean, S.AgeMeanMs, S.TrueAgeMeanMs, S.FirstStateMs, S.Features);
	Row.RemoveFromEnd(TEXT("\n"));
	Row += TEXT(",") + S.Overrides + TEXT("\n");
	FFileHelper::SaveStringToFile(Row, *SummaryPath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
#endif
}

bool URagdollTestSubsystem::IsPelvisAirborne(const ATromboneCharacterBase* Character) const
{
#if WITH_EDITOR
	const FBodyInstance* Pelvis = Character->GetMesh()->GetBodyInstance(TromboneBones::Pelvis);
	if (!Pelvis)
	{
		return false;
	}

	const FVector PelvisLoc = Pelvis->GetUnrealWorldTransform().GetLocation();
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character);
	return !GetWorld()->SweepTestByChannel(
		PelvisLoc,
		PelvisLoc - FVector(0.0f, 0.0f, GroundProbeDistance),
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeSphere(GroundProbeRadius),
		Params);
#else
	return false;
#endif
}

// ---------------------------------------------------------------- campaign

void URagdollTestSubsystem::StartCampaign()
{
#if WITH_EDITOR
	URagdollTestSubsystem* Instance = FindClientInstance();
	if (!Instance)
	{
		UE_LOG(LogTemp, Error, TEXT("[RagdollTest] Needs a PIE client world. Play as Listen Server, 2 players, one process"));
		return;
	}
	Instance->CampaignCases = URagdollTestSettings::Get()->BuildCampaignCases();
	if (Instance->CampaignCases.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[RagdollTest] No cases. Project Settings > Game > Ragdoll Test"));
		return;
	}
	if (Instance->bCampaignActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("[RagdollTest] Campaign already running"));
		return;
	}

	Instance->bCampaignActive = true;
	Instance->Runs.Empty();
	Instance->BeginCase(0);
#endif
}

void URagdollTestSubsystem::StopCampaign()
{
#if WITH_EDITOR
	if (URagdollTestSubsystem* Instance = FindClientInstance(); Instance && Instance->bCampaignActive)
	{
		Instance->EndCampaign(TEXT("stopped"));
	}
#endif
}

void URagdollTestSubsystem::TickCampaign()
{
#if WITH_EDITOR
	const URagdollTestSettings* Settings = URagdollTestSettings::Get();
	if (!CampaignCases.IsValidIndex(CaseIndex))
	{
		EndCampaign(TEXT("case list changed"));
		return;
	}

	const FRagdollTestCase& Case = CampaignCases[CaseIndex];
	const double Now = FApp::GetCurrentTime();

	if (GEngine && Steps.IsValidIndex(StepIndex))
	{
		const FRagdollTestStep& Step = Steps[StepIndex];
		int32 Done = 0, Total = 0;
		for (int32 i = 0; i < Steps.Num(); ++i)
		{
			if (Steps[i].Scenario == Step.Scenario && Steps[i].bRecord == Step.bRecord)
			{
				++Total;
				if (i <= StepIndex) ++Done;
			}
		}
		GEngine->AddOnScreenDebugMessage(CampaignKey, 1.0f, FColor::Green, FString::Printf(TEXT("[RagdollTest] case %d/%d %s | %s%s %d/%d"),
			CaseIndex + 1, CampaignCases.Num(), *Case.Label, Step.bRecord ? TEXT("") : TEXT("warm-up "), ScenarioName(Step.Scenario), Done, Total));
	}

	if (bDropInProgress)
	{
		if (Runs.Num() > 0)
		{
			bDropSeen = true;
		}

		if (bDropSeen && Runs.Num() == 0)
		{
			// Every ragdoll of this step has ended and its summary is in
			bDropInProgress = false;
			++StepIndex;
			NextDropRealTime = Now + Settings->DropInterval;
		}
		else if (!bDropSeen && Now > NextDropRealTime)
		{
			EndCampaign(TEXT("no ragdoll started. Is the lobby waiting in InstrumentScramble?"));
		}
		return;
	}

	if (Now < NextDropRealTime || Runs.Num() > 0)
	{
		return;
	}

	if (!Steps.IsValidIndex(StepIndex))
	{
		FinishCase();
		return;
	}

	TriggerStep(Steps[StepIndex]);
#endif
}

void URagdollTestSubsystem::BeginCase(const int32 InCaseIndex)
{
#if WITH_EDITOR
	const URagdollTestSettings* Settings = URagdollTestSettings::Get();
	RestoreOverrides();
	if (!CampaignCases.IsValidIndex(InCaseIndex))
	{
		EndCampaign(TEXT("done"));
		return;
	}

	CaseIndex = InCaseIndex;
	const FRagdollTestCase& Case = CampaignCases[CaseIndex];

	SetConsoleVariable(TEXT("Trombone.Ragdoll.SyncFeatures"), Case.Features);
	CVarRagdollTestLabel.AsVariable()->Set(*Case.Label, ECVF_SetByConsole);
	ApplyEmulation(Case.PktLag, Case.PktLagVariance, Case.PktLoss);
	ApplyOverrides(Case.Overrides);

	// Warm-ups first, then the two scenarios take turns so a drift in the session hits both alike
	Steps.Empty();
	for (int32 i = 0; i < Settings->WarmupDrops; ++i)
	{
		Steps.Add({ ERagdollTestScenario::Drop, false });
	}
	for (int32 Drops = 0, Launches = 0; Drops < Case.Drops || Launches < Case.Launches;)
	{
		if (Drops < Case.Drops) { Steps.Add({ ERagdollTestScenario::Drop, true }); ++Drops; }
		if (Launches < Case.Launches) { Steps.Add({ ERagdollTestScenario::Launch, true }); ++Launches; }
	}

	StepIndex = 0;
	CaseSummaries.Empty();
	bDropInProgress = false;
	bDropSeen = false;
	NextDropRealTime = FApp::GetCurrentTime() + Settings->DropInterval;

	UE_LOG(LogTemp, Log, TEXT("[RagdollTest] Case %d/%d '%s': features %d, lag %d +-%d ms, loss %d%%, %d drops, %d launches"),
		CaseIndex + 1, CampaignCases.Num(), *Case.Label, Case.Features, Case.PktLag, Case.PktLagVariance, Case.PktLoss, Case.Drops, Case.Launches);
#endif
}

void URagdollTestSubsystem::FinishCase()
{
#if WITH_EDITOR
	const FString Stamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
	for (const ERagdollTestScenario Scenario : { ERagdollTestScenario::Drop, ERagdollTestScenario::Launch })
	{
		for (const bool bSelf : { true, false })
		{
			const FRagdollTestSummary Median = MedianOf(CaseSummaries, ScenarioName(Scenario), bSelf);
			if (Median.Samples == 0)
			{
				continue;
			}

			AppendSummaryRow(Median, Stamp);
			UE_LOG(LogTemp, Log, TEXT("[RagdollTest] %s over %d runs: pelvis mean %.1f p95 %.1f | air %.1f ground %.1f | rot mean %.1f max %.1f | body %.1f | snap %d"),
				*Median.Label, Median.Samples, Median.PelvisMean, Median.PelvisP95, Median.AirMean, Median.GroundMean, Median.RotMean, Median.RotMax, Median.BodyMean, Median.SnapCount);
		}
	}

	BeginCase(CaseIndex + 1);
#endif
}

void URagdollTestSubsystem::TriggerStep(const FRagdollTestStep& Step)
{
#if WITH_EDITOR
	UWorld* Server = FindServerWorld();
	if (!Server)
	{
		EndCampaign(TEXT("no server world"));
		return;
	}

	if (Step.Scenario == ERagdollTestScenario::Drop)
	{
		const AGameModeBase* GameMode = Server->GetAuthGameMode();
		ULobbyDirectorComponent* Director = GameMode ? GameMode->FindComponentByClass<ULobbyDirectorComponent>() : nullptr;
		if (!Director)
		{
			EndCampaign(TEXT("no lobby director on the server. Run this in the lobby"));
			return;
		}
		Director->RelaunchAllPlayersFalling();
	}
	else
	{
		// Standing players thrown up in place, the closest thing to a headbutt hit without a hitter
		for (TActorIterator<ATromboneCharacterBase> It(Server); It; ++It)
		{
			if (UTromboneRagdollComponent* Ragdoll = It->GetRagdollComponent())
			{
				Ragdoll->SetAutoGetUpEnabled(true);
				Ragdoll->StartRagdollLaunched();
			}
		}
	}

	bDropInProgress = true;
	bDropSeen = false;
	NextDropRealTime = FApp::GetCurrentTime() + URagdollTestSettings::Get()->DropTimeout;
#endif
}

const TCHAR* URagdollTestSubsystem::ScenarioName(const ERagdollTestScenario Scenario)
{
	return Scenario == ERagdollTestScenario::Launch ? TEXT("launch") : TEXT("drop");
}

void URagdollTestSubsystem::ApplyEmulation(const int32 PktLag, const int32 PktLagVariance, const int32 PktLoss) const
{
#if WITH_EDITOR && DO_ENABLE_NET_TEST
	for (UWorld* World : { GetWorld(), FindServerWorld() })
	{
		UNetDriver* Driver = World ? World->GetNetDriver() : nullptr;
		if (!Driver)
		{
			continue;
		}

		FPacketSimulationSettings Settings;
		Settings.PktLag = PktLag;
		Settings.PktLagVariance = PktLagVariance;
		Settings.PktLoss = PktLoss;
		Driver->SetPacketSimulationSettings(Settings);
	}
#endif
}

void URagdollTestSubsystem::ApplyOverrides(const TMap<FName, float>& Overrides)
{
#if WITH_EDITOR
	if (Overrides.Num() == 0)
	{
		return;
	}

	// By reflection, so any float UPROPERTY of the component can be swept without a setter for each
	for (UWorld* World : { GetWorld(), FindServerWorld() })
	{
		if (!World)
		{
			continue;
		}

		for (TActorIterator<ATromboneCharacterBase> It(World); It; ++It)
		{
			UTromboneRagdollComponent* Ragdoll = It->GetRagdollComponent();
			if (!Ragdoll)
			{
				continue;
			}

			TMap<FName, float>& Saved = OriginalValues.FindOrAdd(Ragdoll);
			for (const TPair<FName, float>& Pair : Overrides)
			{
				const FFloatProperty* Property = FindFProperty<FFloatProperty>(UTromboneRagdollComponent::StaticClass(), Pair.Key);
				if (!Property)
				{
					UE_LOG(LogTemp, Warning, TEXT("[RagdollTest] No float property '%s' on the ragdoll component, override skipped"), *Pair.Key.ToString());
					continue;
				}

				if (!Saved.Contains(Pair.Key))
				{
					Saved.Add(Pair.Key, Property->GetPropertyValue_InContainer(Ragdoll));
				}
				Property->SetPropertyValue_InContainer(Ragdoll, Pair.Value);
			}
		}
	}
#endif
}

void URagdollTestSubsystem::RestoreOverrides()
{
#if WITH_EDITOR
	for (const TPair<TWeakObjectPtr<UTromboneRagdollComponent>, TMap<FName, float>>& Entry : OriginalValues)
	{
		UTromboneRagdollComponent* Ragdoll = Entry.Key.Get();
		if (!Ragdoll)
		{
			continue;
		}

		for (const TPair<FName, float>& Pair : Entry.Value)
		{
			if (const FFloatProperty* Property = FindFProperty<FFloatProperty>(UTromboneRagdollComponent::StaticClass(), Pair.Key))
			{
				Property->SetPropertyValue_InContainer(Ragdoll, Pair.Value);
			}
		}
	}
	OriginalValues.Empty();
#endif
}

void URagdollTestSubsystem::EndCampaign(const TCHAR* Reason)
{
#if WITH_EDITOR
	UE_LOG(LogTemp, Log, TEXT("[RagdollTest] Campaign ended: %s"), Reason);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(CampaignKey, 8.0f, FColor::Green, FString::Printf(TEXT("[RagdollTest] Campaign ended: %s"), Reason));
	}

	// Leave the session the way a manual test expects it
	RestoreOverrides();
	ApplyEmulation(0, 0, 0);
	SetConsoleVariable(TEXT("Trombone.Ragdoll.SyncFeatures"), static_cast<int32>(ERagdollSyncFeature::All));
	CVarRagdollTestLabel.AsVariable()->Set(TEXT("manual"), ECVF_SetByConsole);

	bCampaignActive = false;
	bDropInProgress = false;
	bDropSeen = false;
	CaseIndex = -1;
	Steps.Empty();
	StepIndex = 0;
	CaseSummaries.Empty();
	CampaignCases.Empty();
#endif
}

FRagdollTestSummary URagdollTestSubsystem::MedianOf(const TArray<FRagdollTestSummary>& InRuns, const FString& Scenario, const bool bSelf) const
{
	FRagdollTestSummary M;
#if WITH_EDITOR
	TArray<const FRagdollTestSummary*> Picked;
	for (const FRagdollTestSummary& S : InRuns)
	{
		if (S.bLocallyControlled == bSelf && S.Scenario == Scenario)
		{
			Picked.Add(&S);
		}
	}
	if (Picked.Num() == 0)
	{
		return M;
	}

	auto Med = [&Picked](float FRagdollTestSummary::* Field)
	{
		TArray<float> Values;
		for (const FRagdollTestSummary* S : Picked)
		{
			Values.Add(S->*Field);
		}
		return Median(Values);
	};
	auto MedInt = [&Picked](int32 FRagdollTestSummary::* Field)
	{
		TArray<float> Values;
		for (const FRagdollTestSummary* S : Picked)
		{
			Values.Add(static_cast<float>(S->*Field));
		}
		return FMath::RoundToInt(Median(Values));
	};

	M.Label = Picked[0]->Label + TEXT("_") + Scenario + (bSelf ? TEXT("_median_self") : TEXT("_median_other"));
	M.Scenario = Scenario;
	M.Overrides = Picked[0]->Overrides;
	M.PlayerName = TEXT("median");
	M.bLocallyControlled = bSelf;
	M.Features = Picked[0]->Features;
	M.Samples = Picked.Num();
	M.Duration = Med(&FRagdollTestSummary::Duration);
	M.SyncTime = Med(&FRagdollTestSummary::SyncTime);
	M.FirstStateMs = Med(&FRagdollTestSummary::FirstStateMs);
	M.PelvisMean = Med(&FRagdollTestSummary::PelvisMean);
	M.PelvisP95 = Med(&FRagdollTestSummary::PelvisP95);
	M.PelvisMax = Med(&FRagdollTestSummary::PelvisMax);
	M.AirMean = Med(&FRagdollTestSummary::AirMean);
	M.GroundMean = Med(&FRagdollTestSummary::GroundMean);
	M.RotMean = Med(&FRagdollTestSummary::RotMean);
	M.RotMax = Med(&FRagdollTestSummary::RotMax);
	M.BodyMean = Med(&FRagdollTestSummary::BodyMean);
	M.BodyMax = Med(&FRagdollTestSummary::BodyMax);
	M.SettleTime = Med(&FRagdollTestSummary::SettleTime);
	M.SnapCount = MedInt(&FRagdollTestSummary::SnapCount);
	M.RecoverCount = MedInt(&FRagdollTestSummary::RecoverCount);
	M.FpsMean = Med(&FRagdollTestSummary::FpsMean);
	M.PingMean = Med(&FRagdollTestSummary::PingMean);
	M.AgeMeanMs = Med(&FRagdollTestSummary::AgeMeanMs);
	M.TrueAgeMeanMs = Med(&FRagdollTestSummary::TrueAgeMeanMs);
#endif
	return M;
}
