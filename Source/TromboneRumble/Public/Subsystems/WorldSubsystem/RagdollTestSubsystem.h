// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RagdollTestSubsystem.generated.h"

class ATromboneCharacterBase;
class UTromboneRagdollComponent;
struct FRagdollTestCase;

/** How a ragdoll gets started in a test. */
enum class ERagdollTestScenario : uint8
{
	/** Lobby fall from the drop points. Vertical, fast, hidden until the first state. */
	Drop,

	/** Thrown up from standing, like a headbutt hit. Visible the whole time. */
	Launch,
};

/** One ragdoll of a campaign case. */
struct FRagdollTestStep
{
	ERagdollTestScenario Scenario = ERagdollTestScenario::Drop;

	/** False for warm-ups: measured and printed, not written. */
	bool bRecord = true;
};

/** One frame of measurement. Client body vs the server body of the same PIE session. */
struct FRagdollTestSample
{
	/** Seconds since the ragdoll started. */
	float Time = 0.0f;

	float Fps = 0.0f;

	/** Estimated physics step: frame dt capped by the physics settings. */
	float PhysicsDt = 0.0f;

	float PingMs = 0.0f;

	bool bAirborne = false;

	/** Client pelvis vs server pelvis (cm). */
	float PelvisError = 0.0f;

	/** Client pelvis vs server pelvis (deg). */
	float PelvisRotError = 0.0f;

	/** Mean over every body (cm). */
	float BodyError = 0.0f;

	/** Server pelvis speed (cm/s). */
	float ServerSpeed = 0.0f;

	/** Client pelvis vs the target it steers to (cm). */
	float TargetError = 0.0f;

	/** What the component thinks the state age is (s). */
	float PacketAge = 0.0f;

	/** Real age of the state: server clock minus its timestamp (s). Only PIE can know this. */
	float TrueAge = 0.0f;

	/** Velocity correction added by the P-control this frame (cm/s). */
	float CorrectionSpeed = 0.0f;
};

/** Every sample of one ragdoll, from start to get-up. */
struct FRagdollTestRun
{
	/** Set once BeginRun has looked for the server character, found or not. */
	bool bStarted = false;

	TWeakObjectPtr<ATromboneCharacterBase> ServerCharacter;

	FString PlayerName;

	bool bLocallyControlled = false;

	/** Client world time when the ragdoll started. */
	float StartTime = 0.0f;

	/** Seconds from the start until the client used its first state of this ragdoll. -1 while waiting. */
	float FirstStateTime = -1.0f;

	TArray<FRagdollTestSample> Samples;

	int32 SnapCount = 0;

	int32 PenetrationRecoveryCount = 0;

	/** Running max of PelvisError for the overlay. */
	float MaxPelvisError = 0.0f;
};

/** The numbers of one finished run. One summary line, one summary.csv row. */
struct FRagdollTestSummary
{
	FString Label;

	/** drop, launch, or manual outside a campaign. */
	FString Scenario;

	/** Parameter overrides of the case, "Name=Value;..." or empty. */
	FString Overrides;
	FString PlayerName;
	bool bLocallyControlled = false;
	int32 Features = 0;

	/** Samples counted, after the catch-up was cut off. */
	int32 Samples = 0;
	float Duration = 0.0f;
	float SyncTime = 0.0f;
	float FirstStateMs = 0.0f;

	float PelvisMean = 0.0f;
	float PelvisP95 = 0.0f;
	float PelvisMax = 0.0f;
	float AirMean = 0.0f;
	float GroundMean = 0.0f;
	float RotMean = 0.0f;
	float RotMax = 0.0f;
	float BodyMean = 0.0f;
	float BodyMax = 0.0f;
	float SettleTime = -1.0f;
	int32 SnapCount = 0;
	int32 RecoverCount = 0;
	float FpsMean = 0.0f;
	float PingMean = 0.0f;
	float AgeMeanMs = 0.0f;
	float TrueAgeMeanMs = 0.0f;
};

/** URagdollTestSubsystem
 *  Measures how far the client ragdoll is from the server one. PIE only.
 *  Both worlds live in one process, so the server body is read directly. No extra traffic, no self-scoring.
 *
 *  Trombone.Ragdoll.Test 0 off / 1 overlay / 2 overlay + CSV in Saved/RagdollTest.
 *  Trombone.Ragdoll.Test.Label tags the summary rows.
 *  Trombone_RagdollTest runs every case in URagdollTestSettings by itself: sets the features and the fake latency,
 *  drops the players again and again, and writes median rows per case.
 */
UCLASS()
class TROMBONERUMBLE_API URagdollTestSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:

	/** Starts the campaign on the PIE client world. Works from any PIE window. */
	static void StartCampaign();

	static void StopCampaign();

	// ~ Begin USubsystem Interface
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Deinitialize() override;
	// ~ End USubsystem Interface

	// ~ Begin FTickableGameObject Interface
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	// ~ End FTickableGameObject Interface

private:

	/** @return The listen server world of this PIE session, or null. */
	UWorld* FindServerWorld() const;

	/** @return The subsystem of the PIE client world, or null. */
	static URagdollTestSubsystem* FindClientInstance();

	/** 0 off, 1 overlay, 2 overlay + CSV. A running campaign always records. */
	int32 GetMode() const;

	/** Pairs the client character with the server one by player id. */
	void BeginRun(FRagdollTestRun& Run, const ATromboneCharacterBase* Character, UWorld* Server) const;

	void Sample(FRagdollTestRun& Run, const ATromboneCharacterBase* Character, float DeltaTime) const;

	void DrawOverlay(const FRagdollTestRun& Run, const ATromboneCharacterBase* Character, int32 Index, float DeltaTime) const;

	/** Summarizes, prints, writes the CSV files when recording, and hands the summary to the campaign. */
	void EndRun(const FRagdollTestRun& Run);

	FRagdollTestSummary Summarize(const FRagdollTestRun& Run) const;

	void PrintSummary(const FRagdollTestSummary& S) const;

	void WriteRunCsv(const FRagdollTestRun& Run, const FRagdollTestSummary& S, const FString& Stamp) const;

	void AppendSummaryRow(const FRagdollTestSummary& S, const FString& Stamp) const;

	bool IsPelvisAirborne(const ATromboneCharacterBase* Character) const;

	// ~ Begin campaign
	void TickCampaign();

	/** Applies the case, then waits one interval before the first drop. */
	void BeginCase(int32 InCaseIndex);

	/** Writes the median rows of the case and moves on. */
	void FinishCase();

	/** Starts the ragdolls of one step on the server world. */
	void TriggerStep(const FRagdollTestStep& Step);

	/** Fake latency on both PIE net drivers. Zero clears it. */
	void ApplyEmulation(int32 PktLag, int32 PktLagVariance, int32 PktLoss) const;

	/** Sets the case's float properties on every ragdoll component of both worlds, remembering the old values. */
	void ApplyOverrides(const TMap<FName, float>& Overrides);

	/** Puts back every value ApplyOverrides changed. */
	void RestoreOverrides();

	void EndCampaign(const TCHAR* Reason);

	/** Median of every summary field over the runs of a case with this scenario and role. */
	FRagdollTestSummary MedianOf(const TArray<FRagdollTestSummary>& Runs, const FString& Scenario, bool bSelf) const;

	static const TCHAR* ScenarioName(ERagdollTestScenario Scenario);
	// ~ End campaign

private:

	TMap<TWeakObjectPtr<ATromboneCharacterBase>, FRagdollTestRun> Runs;

	mutable TWeakObjectPtr<UWorld> ServerWorld;

	/** Campaign state. Idle unless Trombone_RagdollTest is running. */
	bool bCampaignActive = false;

	/** Cases plus expanded sweeps, fixed at campaign start. */
	TArray<FRagdollTestCase> CampaignCases;

	/** Property values as they were before the current case's overrides, per component. */
	TMap<TWeakObjectPtr<UTromboneRagdollComponent>, TMap<FName, float>> OriginalValues;

	int32 CaseIndex = -1;

	/** Warm-ups first, then drops and launches taking turns. */
	TArray<FRagdollTestStep> Steps;

	int32 StepIndex = 0;

	/** True between a drop being triggered and its ragdolls all ending. */
	bool bDropInProgress = false;

	/** Set once a ragdoll of the current drop has been seen. */
	bool bDropSeen = false;

	/** Real time the next drop may go, or the drop deadline while one is in progress. */
	double NextDropRealTime = 0.0;

	/** Summaries of the recorded runs of the current case. */
	TArray<FRagdollTestSummary> CaseSummaries;
};
