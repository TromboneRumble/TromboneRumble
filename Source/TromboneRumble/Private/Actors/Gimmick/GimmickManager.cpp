// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Actors/Gimmick/GimmickManager.h"
#include "EngineUtils.h"
#include "Actors/Gimmick/GimmickBase.h"
#include "Data/Gimmick/GimmickConfig.h"
#include "Data/Gimmick/StageGimmickData.h"
#include "Engine/Engine.h"
#include "Subsystems/RhythmSubsystem.h"
#include "TimerManager.h"
#include "Utilities/EnumHelper.h"
#include "Utilities/TromboneLogs.h"

#if WITH_EDITOR
#include "Logging/MessageLog.h"
#include "Misc/UObjectToken.h"
#endif

#if !UE_BUILD_SHIPPING
namespace GimmickConsole
{
	enum class ECommand : uint8 { Trigger, Stop, Restart };

	/** Run one command on the manager of the world the console belongs to. */
	static void Run(const ECommand Command, const TArray<FString>& Args, UWorld* World)
	{
		EGimmickType GimmickType = EGimmickType::None;
		if (Args.Num() < 1 || !EnumHelper::StringToEnum(Args[0], GimmickType))
		{
			UE_LOG(LogGimmick, Warning, TEXT("Give a gimmick type, for example Gravity"));
			return;
		}

		TActorIterator<AGimmickManager> It(World);
		AGimmickManager* Manager = It ? *It : nullptr;
		if (!Manager || !Manager->HasAuthority())
		{
			UE_LOG(LogGimmick, Warning, TEXT("No gimmick manager with authority in this world. Run the command on the host"));
			return;
		}

		switch (Command)
		{
		case ECommand::Trigger:
			Manager->ForceTriggerGimmick(GimmickType);
			break;
		case ECommand::Stop:
			Manager->DeactivateGimmick(GimmickType);
			break;
		case ECommand::Restart:
			Manager->DeactivateGimmick(GimmickType);
			Manager->ActivateGimmick(GimmickType);
			break;
		}
	}

	static FAutoConsoleCommandWithWorldAndArgs TriggerCommand(
		TEXT("Trombone.Gimmick.Trigger"),
		TEXT("기믹을 타이머를 기다리지 않고 바로 발동. 예: Trombone.Gimmick.Trigger Gravity"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World) { Run(ECommand::Trigger, Args, World); }));

	static FAutoConsoleCommandWithWorldAndArgs StopCommand(
		TEXT("Trombone.Gimmick.Stop"),
		TEXT("기믹을 끔. 예: Trombone.Gimmick.Stop Gravity"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World) { Run(ECommand::Stop, Args, World); }));

	static FAutoConsoleCommandWithWorldAndArgs RestartCommand(
		TEXT("Trombone.Gimmick.Restart"),
		TEXT("기믹을 껐다 켜서 바뀐 설정을 바로 반영. 예: Trombone.Gimmick.Restart Gravity"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World) { Run(ECommand::Restart, Args, World); }));
}
#endif

void AGimmickManager::ActivateGimmick(const EGimmickType GimmickType)
{
	if (AGimmickBase* Gimmick = FindGimmick(GimmickType))
	{
		Gimmick->Activate();
	}
}

void AGimmickManager::DeactivateGimmick(const EGimmickType GimmickType)
{
	if (AGimmickBase* Gimmick = FindGimmick(GimmickType))
	{
		Gimmick->Deactivate();
	}
}

void AGimmickManager::ActivateAllGimmicks()
{
	UE_LOG(LogGimmick, Log, TEXT("Activating %d gimmicks"), ManagedGimmicks.Num());

	for (auto& Pair : ManagedGimmicks)
	{
		// A gimmick turned on earlier, for example by a forced trigger, keeps running
		// Some gimmicks bind delegates and spawn in Activate without checking IsActive, so we never call it twice
		if (Pair.Value && !Pair.Value->IsActive())
		{
			Pair.Value->Activate();
		}
	}

	StartSequences();
}

void AGimmickManager::DeactivateAllGimmicks()
{
	UE_LOG(LogGimmick, Log, TEXT("Deactivating %d gimmicks"), ManagedGimmicks.Num());

	// First, so the events the gimmicks end while turning off do not start another turn
	StopSequences();

	for (auto& Pair : ManagedGimmicks)
	{
		if (Pair.Value)
		{
			Pair.Value->Deactivate();
		}
	}
}

void AGimmickManager::ForceTriggerGimmick(const EGimmickType GimmickType)
{
	AGimmickBase* Gimmick = FindGimmick(GimmickType);
	if (!Gimmick)
	{
		UE_LOG(LogGimmick, Warning, TEXT("This level has no gimmick of type %s"), *EnumHelper::EnumToString(GimmickType));
		return;
	}

	// We turn it on first, because a gimmick that is off would not clear the timers ForceTrigger starts
	// Activate of some gimmicks binds delegates and spawns without checking IsActive, so we never call it twice
	if (!Gimmick->IsActive())
	{
		Gimmick->Activate();
	}
	Gimmick->ForceTrigger();
}

AGimmickManager* AGimmickManager::Find(const UWorld* World)
{
	if (!World) return nullptr;

	TActorIterator<AGimmickManager> It(World);
	return It ? *It : nullptr;
}

void AGimmickManager::StartSequences()
{
	StopSequences();
	if (!HasAuthority() || !StageData) return;

	const TArray<FGimmickSequence>& Sequences = StageData->GetSequences();
	for (int32 SequenceIndex = 0; SequenceIndex < Sequences.Num(); ++SequenceIndex)
	{
		if (Sequences[SequenceIndex].Order.IsEmpty()) continue;

		const int32 RunIndex = SequenceRuns.AddDefaulted();
		SequenceRuns[RunIndex].SequenceIndex = SequenceIndex;
		ScheduleNextInSequence(RunIndex, Sequences[SequenceIndex].FirstDelay);
	}
}

void AGimmickManager::StopSequences()
{
	for (FSequenceRun& Run : SequenceRuns)
	{
		GetWorldTimerManager().ClearTimer(Run.TimerHandle);
	}
	SequenceRuns.Reset();
}

void AGimmickManager::ScheduleNextInSequence(const int32 RunIndex, const float Delay)
{
	FSequenceRun& Run = SequenceRuns[RunIndex];
	Run.Running = EGimmickType::None;
	GetWorldTimerManager().SetTimer(Run.TimerHandle, FTimerDelegate::CreateUObject(this, &ThisClass::StartNextInSequence, RunIndex), Delay, false);
}

void AGimmickManager::StartNextInSequence(const int32 RunIndex)
{
	if (!SequenceRuns.IsValidIndex(RunIndex) || !StageData) return;

	FSequenceRun& Run = SequenceRuns[RunIndex];
	const FGimmickSequence& Sequence = StageData->GetSequences()[Run.SequenceIndex];

	const EGimmickType Type = Sequence.Order[Run.NextIndex];
	Run.NextIndex = (Run.NextIndex + 1) % Sequence.Order.Num();

	// A missing or stopped gimmick must not stall the others, so its turn passes to the next one
	AGimmickBase* Gimmick = FindGimmick(Type);
	if (!Gimmick || !Gimmick->IsActive())
	{
		UE_LOG(LogGimmick, Warning, TEXT("Sequence skips %s, the level has no such gimmick or it is off"), *EnumHelper::EnumToString(Type));
		ScheduleNextInSequence(RunIndex, Sequence.Gap);
		return;
	}

	UE_LOG(LogGimmick, Log, TEXT("Sequence starts %s"), *EnumHelper::EnumToString(Type));
	Run.Running = Type;
	Gimmick->ForceTrigger();
}

void AGimmickManager::HandleEventFinished(const AGimmickBase& Gimmick)
{
	if (!HasAuthority() || !StageData) return;

	for (int32 RunIndex = 0; RunIndex < SequenceRuns.Num(); ++RunIndex)
	{
		// A forced event outside the turn also ends here. Only the gimmick whose turn it is moves the sequence on
		if (SequenceRuns[RunIndex].Running != Gimmick.GetGimmickType()) continue;

		ScheduleNextInSequence(RunIndex, StageData->GetSequences()[SequenceRuns[RunIndex].SequenceIndex].Gap);
		return;
	}
}

AGimmickBase* AGimmickManager::FindGimmick(const EGimmickType GimmickType) const
{
	const TObjectPtr<AGimmickBase>* Found = ManagedGimmicks.Find(GimmickType);
	return Found ? Found->Get() : nullptr;
}

TSet<EGimmickType> AGimmickManager::FindGimmickTypesInLevel() const
{
	TSet<EGimmickType> Types;
	for (TActorIterator<AGimmickBase> It(GetWorld()); It; ++It)
	{
		Types.Add(It->GetGimmickType());
	}
	return Types;
}

void AGimmickManager::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		SpawnMissingGimmicks();
	}
	RegisterLevelGimmicks();
	ValidateStageData();

	if (UWorld* World = GetWorld())
	{
		if (AInGameState* GameState = Cast<AInGameState>(GetWorld()->GetGameState()))
		{
			GameState->OnInGameStateChanged.AddDynamic(this, &ThisClass::HandleInGameStateChanged);
		}
		else
		{
			World->GameStateSetEvent.AddUObject(this, &ThisClass::HandleGameStateSet);
		}
	}
	if (HasAuthority())
	{
		if (URhythmSubsystem* RS = GetGameInstance()->GetSubsystem<URhythmSubsystem>())
		{
			RS->OnMusicCallback.AddDynamic(this, &ThisClass::HandleMusicCallback);
		}
	}
}

const UGimmickConfig* AGimmickManager::FindConfigInWorld(const UWorld* World, const EGimmickType GimmickType)
{
	if (!World) return nullptr;

	for (TActorIterator<AGimmickManager> It(World); It; ++It)
	{
		if (const UStageGimmickData* Data = It->GetStageData())
		{
			return Data->FindConfig(GimmickType);
		}
	}
	return nullptr;
}

void AGimmickManager::SpawnMissingGimmicks()
{
	UWorld* World = GetWorld();
	if (!World || !StageData) return;

	TSet<EGimmickType> TypesInLevel = FindGimmickTypesInLevel();

	for (const UGimmickConfig* Config : StageData->GetGimmicks())
	{
		if (!Config || !Config->GetGimmickClass() || TypesInLevel.Contains(Config->GetGimmickType())) continue;

		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (World->SpawnActor<AGimmickBase>(Config->GetGimmickClass(), GetActorTransform(), Params))
		{
			TypesInLevel.Add(Config->GetGimmickType());
			UE_LOG(LogGimmick, Log, TEXT("Spawned %s because the level has no gimmick of type %s"),
				*Config->GetGimmickClass()->GetName(), *EnumHelper::EnumToString(Config->GetGimmickType()));
		}
	}
}

TArray<const UGimmickConfig*> AGimmickManager::FindUnusedConfigs() const
{
	TArray<const UGimmickConfig*> Unused;
	if (!GetWorld() || !StageData) return Unused;

	const TSet<EGimmickType> TypesInLevel = FindGimmickTypesInLevel();

	for (const UGimmickConfig* Config : StageData->GetGimmicks())
	{
		if (Config && !Config->GetGimmickClass() && !TypesInLevel.Contains(Config->GetGimmickType()))
		{
			Unused.Add(Config);
		}
	}
	return Unused;
}

TArray<const UGimmickConfig*> AGimmickManager::FindConfigsWithoutSequence() const
{
	TArray<const UGimmickConfig*> Result;
	if (!StageData) return Result;

	for (const UGimmickConfig* Config : StageData->GetGimmicks())
	{
		if (Config && Config->RunsOnlyInSequence() && !StageData->FindSequence(Config->GetGimmickType()))
		{
			Result.Add(Config);
		}
	}
	return Result;
}

void AGimmickManager::ValidateStageData() const
{
	// A client would repeat the message of the host
	if (!HasAuthority()) return;

	const auto Warn = [](const FString& Message)
	{
		UE_LOG(LogGimmick, Warning, TEXT("%s"), *Message);
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.f, FColor::Yellow, Message);
		}
#endif
	};

	// Without stage data the gimmicks run on class defaults
	// Garbage and presents have empty class lists there, so nothing spawns
	if (!StageData && ManagedGimmicks.Num() > 0)
	{
		Warn(FString::Printf(TEXT("[기믹] %s 에 레벨 기믹 설정이 비어 있습니다. 기믹 %d개가 기본값으로 동작하고, 투척물과 선물, 순서 그룹이 필요한 기믹은 나오지 않습니다"),
			*GetName(), ManagedGimmicks.Num()));
		return;
	}

	for (const UGimmickConfig* Config : FindUnusedConfigs())
	{
		Warn(FString::Printf(TEXT("[기믹] %s 에 %s 항목이 있지만 레벨에 그 기믹이 없습니다. 기믹을 배치하거나 항목을 지우세요"),
			*StageData->GetName(), *UEnum::GetDisplayValueAsText(Config->GetGimmickType()).ToString()));
	}

	for (const UGimmickConfig* Config : FindConfigsWithoutSequence())
	{
		Warn(FString::Printf(TEXT("[기믹] %s 은(는) 순서 그룹에서만 발동하는데 %s 의 어느 순서 그룹에도 없어 나오지 않습니다"),
			*UEnum::GetDisplayValueAsText(Config->GetGimmickType()).ToString(), *StageData->GetName()));
	}
}

#if WITH_EDITOR
void AGimmickManager::CheckForErrors()
{
	Super::CheckForErrors();

	if (!StageData)
	{
		FMessageLog("MapCheck").Warning()
			->AddToken(FUObjectToken::Create(this))
			->AddToken(FTextToken::Create(FText::FromString(TEXT("레벨 기믹 설정이 비어 있습니다. 이 레벨의 기믹은 기본값으로 동작하고, 투척물과 선물, 순서 그룹이 필요한 기믹은 나오지 않습니다"))));
		return;
	}

	for (const UGimmickConfig* Config : FindConfigsWithoutSequence())
	{
		FMessageLog("MapCheck").Warning()
			->AddToken(FUObjectToken::Create(this))
			->AddToken(FTextToken::Create(FText::FromString(FString::Printf(TEXT("%s 은(는) 순서 그룹에서만 발동하는데 %s 의 어느 순서 그룹에도 없습니다"),
				*UEnum::GetDisplayValueAsText(Config->GetGimmickType()).ToString(), *StageData->GetName()))));
	}

	for (const UGimmickConfig* Config : FindUnusedConfigs())
	{
		FMessageLog("MapCheck").Warning()
			->AddToken(FUObjectToken::Create(this))
			->AddToken(FTextToken::Create(FText::FromString(FString::Printf(TEXT("%s 에 %s 항목이 있지만 레벨에 그 기믹이 없습니다"),
				*StageData->GetName(), *UEnum::GetDisplayValueAsText(Config->GetGimmickType()).ToString()))));
	}
}
#endif

void AGimmickManager::RegisterLevelGimmicks()
{
	ManagedGimmicks.Empty();
	
	for (TActorIterator<AGimmickBase> It(GetWorld()); It; ++It)
	{
		AGimmickBase* Gimmick = *It;

		if (ManagedGimmicks.Contains(Gimmick->GetGimmickType()))
		{
			UE_LOG(LogGimmick, Warning, TEXT("%s: type %s is already registered. The earlier gimmick is replaced"),
				*Gimmick->GetName(), *EnumHelper::EnumToString(Gimmick->GetGimmickType()));
		}

		ManagedGimmicks.Add(Gimmick->GetGimmickType(), Gimmick);
	}

	UE_LOG(LogGimmick, Log, TEXT("Registered %d gimmicks"), ManagedGimmicks.Num());
}

void AGimmickManager::HandleGameStateSet(AGameStateBase* NewGameState)
{
	if (AInGameState* GameState = Cast<AInGameState>(NewGameState))
	{
		GameState->OnInGameStateChanged.RemoveDynamic(this, &ThisClass::HandleInGameStateChanged);
		GameState->OnInGameStateChanged.AddDynamic(this, &ThisClass::HandleInGameStateChanged);
	}
}

void AGimmickManager::HandleInGameStateChanged(EInGameState InGameState)
{
	if (InGameState == EInGameState::End)
	{
		DeactivateAllGimmicks();
	}
}

void AGimmickManager::HandleMusicCallback(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo)
{
	if (!HasAuthority()) return;
	if (const UAkMusicSyncCallbackInfo* MusicInfo = Cast<UAkMusicSyncCallbackInfo>(CallbackInfo))
	{
		const FString& CueString = MusicInfo->UserCueName;
		if (!CueString.IsEmpty())
		{
			const FName CueName(*CueString);
			// Only turns on. A gimmick forced before the cue keeps running instead of being cut off
			if (CueName == TEXT("Event_Spotlight_Start"))
			{
				ActivateAllGimmicks();
			}
		}
	}
}