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
	Trombone,
	Cymbals,
	Violin,
	Audience,
	Invalid UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ENoteResult : uint8
{
	None        UMETA(DisplayName = "None"), // 롱노트 시작, 혹은 롱노트 중간점 반환용
	Bad         UMETA(DisplayName = "Bad"), //미스 판정용
	Good        UMETA(DisplayName = "Good"),
	Great       UMETA(DisplayName = "Great"),
	Excellent   UMETA(DisplayName = "Excellent"),
	Invalid     UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EInstrumentType : uint8
{
	Trombone    UMETA(DisplayName = "Trombone"),
	Violin		UMETA(DisplayName = "Violin"),
	Cymbal		UMETA(DisplayName = "Cymbal"),
	Background	UMETA(DisplayName = "Background"), // Instrument 추가시 Background 위에다 추가할 것
	Invalid		UMETA(Hidden)
};

USTRUCT()
struct FViewportChangedMessage
{
	GENERATED_BODY()

public:
	UPROPERTY()
	float LaneXStartPos = 0.f;

	UPROPERTY()
	float LaneXEndPos = 0.f;

	UPROPERTY()
	TArray<float> LaneYPosArray;
};
