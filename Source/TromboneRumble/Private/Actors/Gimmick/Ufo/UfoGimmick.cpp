// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Actors/Gimmick/Ufo/UfoGimmick.h"
#include "Actors/Gimmick/Ufo/Ufo.h"
#include "Data/Gimmick/UfoGimmickConfig.h"
#include "Engine/TargetPoint.h"
#include "Net/UnrealNetwork.h"
#include "Utilities/TromboneLogs.h"
#include "Utilities/TromboneStatics.h"

#if WITH_EDITOR
#include "Logging/MessageLog.h"
#include "Misc/UObjectToken.h"
#endif

AUfoGimmick::AUfoGimmick()
{
	GimmickType = EGimmickType::Ufo;
	bReplicates = true;
}

const UEventGimmickConfig& AUfoGimmick::GetEventConfig() const
{
	return GetUfoConfig();
}

bool AUfoGimmick::PrepareEvent()
{
	// The place is picked now so the warning can show it
	if (!PickPath(PlannedPath))
	{
		UE_LOG(LogGimmick, Warning, TEXT("[UFO] %s has no line. Set the line list on it"), *GetName());
		return false;
	}
	return true;
}

void AUfoGimmick::OnWarningStarted()
{
	SetState(EUfoState::Warning);
}

void AUfoGimmick::OnEventPhaseStarted(const int32 PhaseIndex)
{
	// The UFO ends the event itself when it is gone, so there is only phase 0
	if (!UfoClass)
	{
		UE_LOG(LogGimmick, Warning, TEXT("[UFO] %s has no UfoClass"), *GetName());
		FinishEvent();
		return;
	}

	PlannedPath.StartServerTime = UTromboneStatics::GetServerWorldTime(this);

	AUfo* Ufo = GetWorld()->SpawnActorDeferred<AUfo>(UfoClass, FTransform(PlannedPath.Start), this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!Ufo)
	{
		FinishEvent();
		return;
	}

	Ufo->InitPath(PlannedPath);
	Ufo->FinishSpawning(FTransform(PlannedPath.Start));
	Ufo->OnDestroyed.AddDynamic(this, &ThisClass::HandleUfoDestroyed);
	ActiveUfo = Ufo;

	SetState(EUfoState::Active);
}

void AUfoGimmick::HandleUfoDestroyed(AActor* DestroyedActor)
{
	ActiveUfo = nullptr;

	if (!IsActive()) return;

	FinishEvent();
}

void AUfoGimmick::OnEventEnded()
{
	SetState(EUfoState::Idle);
}

void AUfoGimmick::OnEventStopped()
{
	if (ActiveUfo)
	{
		ActiveUfo->OnDestroyed.RemoveDynamic(this, &ThisClass::HandleUfoDestroyed);
		ActiveUfo->Destroy();
		ActiveUfo = nullptr;
	}
	SetState(EUfoState::Idle);
}

bool AUfoGimmick::PickPath(FUfoPath& OutPath) const
{
	// A line whose two points sit on the same spot has no length, and a UFO on it would be done at once
	TArray<const FUfoLine*> ValidLines;
	for (const FUfoLine& Line : Lines)
	{
		if (Line.IsValid() && FVector::DistSquared2D(Line.Start->GetActorLocation(), Line.End->GetActorLocation()) > 1.f)
		{
			ValidLines.Add(&Line);
		}
	}
	if (ValidLines.IsEmpty()) return false;

	const FUfoLine& Line = *ValidLines[FMath::RandRange(0, ValidLines.Num() - 1)];
	FVector Start = Line.Start->GetActorLocation();
	FVector End = Line.End->GetActorLocation();

	// Either direction, so the same line does not always read the same way
	if (FMath::RandBool())
	{
		Swap(Start, End);
	}

	// The height of the points does not count. The UFO flies where its beam just reaches the floor
	const UUfoGimmickConfig& Config = GetConfig<UUfoGimmickConfig>();
	Start.Z = Config.GetFlightZ();
	End.Z = Config.GetFlightZ();

	OutPath.Start = Start;
	OutPath.End = End;
	OutPath.Duration = FVector::Dist(Start, End) / Config.MoveSpeed;
	return true;
}

void AUfoGimmick::SetState(const EUfoState NewState)
{
	if (!HasAuthority() || State == NewState) return;

	State = NewState;
	OnUfoStateChanged(State);

	// The warning is short, so the clients should not wait for the next scheduled update
	ForceNetUpdate();
}

void AUfoGimmick::OnRep_State()
{
	OnUfoStateChanged(State);
}

void AUfoGimmick::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, State);
	DOREPLIFETIME(ThisClass, PlannedPath);
}

float AUfoGimmick::GetAverageLineLength() const
{
	float Total = 0.f;
	int32 Count = 0;
	for (const FUfoLine& Line : Lines)
	{
		if (!Line.IsValid()) continue;

		Total += FVector::Dist2D(Line.Start->GetActorLocation(), Line.End->GetActorLocation());
		++Count;
	}
	return Count > 0 ? Total / Count : 0.f;
}

#if WITH_EDITOR
void AUfoGimmick::CheckForErrors()
{
	Super::CheckForErrors();

	const bool bHasLine = Lines.ContainsByPredicate([](const FUfoLine& Line) { return Line.IsValid(); });
	if (!bHasLine)
	{
		FMessageLog("MapCheck").Warning()
			->AddToken(FUObjectToken::Create(this))
			->AddToken(FTextToken::Create(FText::FromString(TEXT("미니 UFO: 이동 라인이 없어 UFO가 나오지 않습니다"))));
	}
}
#endif
