#pragma once

#include "CoreMinimal.h"
#include "Defines.generated.h"

UENUM()
enum class EEquipmentSlotType : uint8
{
	Instrument,
	MAX_SLOTS
};

// TODO : 추후 팀전 고려.
UENUM()
enum class EMatchState : uint8
{
	FreeForAll,
	TwoTeams,
	Invalid = 255 UMETA(Hidden)
};

UENUM()
enum class EGameState : uint8
{
	MainMenu,
	Lobby,
	InGame,
	Invalid = 255 UMETA(Hidden)
};

UENUM()
enum class ELobbyState : uint8
{
	WaitingForPlayers,
	CountdownToScramble,
	InstrumentScramble,
	CountdownToTravel,
	Invalid					= 255 UMETA(Hidden)
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
	Invalid		= 255	UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ENoteResult : uint8
{
	None        = 0		UMETA(DisplayName = "None"), // 롱노트 시작, 혹은 롱노트 중간점 반환용
	Bad         = 1		UMETA(DisplayName = "Bad"),  //미스 판정용
	Good        = 2		UMETA(DisplayName = "Good"),
	Excellent   = 3		UMETA(DisplayName = "Excellent"),
	Invalid		= 255   UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EInstrumentType : uint8
{
	Background	= 0		UMETA(DisplayName = "Background"),
	Trombone    = 1		UMETA(DisplayName = "Trombone"),
	Violin		= 2		UMETA(DisplayName = "Violin"),
	Cymbal		= 3		UMETA(DisplayName = "Cymbal"),

	None 		= 254   UMETA(DisplayName = "None"),
	Invalid		= 255	UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FNoteHandle
{
	GENERATED_BODY()
	UPROPERTY()
	FGuid Id = FGuid::NewGuid();
	UPROPERTY()
	TWeakObjectPtr<AActor> NoteActor = nullptr;
};

USTRUCT()
struct FNoteJudgedMessage
{
	GENERATED_BODY()
	UPROPERTY() FNoteHandle Handle;
	UPROPERTY() EInstrumentType Instrument = EInstrumentType::Invalid;
	UPROPERTY() ENoteResult Judge = ENoteResult::None;
};

USTRUCT()
struct FComboChangedMessage
{
	GENERATED_BODY()
	UPROPERTY() int32 Combo = 0;
	UPROPERTY() int32 DeltaScore = 0;
};

USTRUCT()
struct FViewportChangedMessage
{
	GENERATED_BODY()

	UPROPERTY()	float LaneXStartPos = 0.f;
	UPROPERTY()	float LaneXEndPos = 0.f;
	UPROPERTY()	TArray<float> LaneYPosArray;
};
