#pragma once

#include "CoreMinimal.h"
#include "Defines.generated.h"

USTRUCT()
struct FInteractionContext
{
	GENERATED_BODY()

public:
	bool bIsEquipped = false;

	FInteractionContext() {}
};

// TODO : 추후 팀전 고려.
UENUM()
enum class EMatchState : uint8
{
	FreeForAll,
	TwoTeams,
	Invalid UMETA(Hidden)
};

UENUM()
enum class EGameState : uint8
{
	MainMenu,
	Lobby,
	InGame,
	Invalid UMETA(Hidden)
};

UENUM()
enum class ELobbyState : uint8
{
	WaitingForPlayers,
	CountdownToScramble,
	InstrumentScramble,
	CountdownToTravel,
	Invalid UMETA(Hidden)
};

UENUM()
enum class EHitType : uint8
{
	Headbutt,
	Instrument,
	Invalid UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ENoteResult : uint8
{
	None        UMETA(DisplayName = "None"),
	Bad         UMETA(DisplayName = "Bad"),
	Good        UMETA(DisplayName = "Good"),
	Great       UMETA(DisplayName = "Great"),
	Excellent   UMETA(DisplayName = "Excellent"),
	Invalid     UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FRhythmTraceResult
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly	, VisibleAnywhere, Category = "Result")
	TWeakObjectPtr<class ARhythmNote> NoteActor = nullptr;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Result")
	ENoteResult Judge = ENoteResult::None;

	FRhythmTraceResult() {}
};

