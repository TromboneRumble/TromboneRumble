#pragma once

#include "CoreMinimal.h"
#include "Defines.generated.h"

UENUM()
enum class EMenuBGMType : uint8
{
	BGM_0,
	BGM_1,
	BGM_2,
	MAX,
	None = 255,
};

UENUM()
enum class EGimmickType : uint8
{
	Spotlight,
	Puddle,
	Trash,
	MAX,
	None = 255,
};

UENUM()
enum class ECharacterFaceState : uint8
{
	Blink,
	Stun,
	Ragdoll,
	Victory,
	Lose,
	Hit,
	None = 255,
};

UENUM()
enum class ECharacterFaceType : uint8
{
	Blink0 = 0,
	Blink1 = 1,
	Blink2 = 2,
	Stun = 3,
	Ragdoll0 = 4,
	Ragdoll1 = 5,
	Ragdoll2 = 6,
	Ragdoll3 = 7,
	None = 255,
};

UENUM()
enum class EEquipmentSlotType : uint8
{
	Weapon,
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
enum class ELevelState : uint8
{
	MainMenu,
	Lobby,
	InGame,
	MatchMenu,
	Invalid = 255 UMETA(Hidden)
};

UENUM()
enum class EInGameState : uint8
{
	Initializing,
	Play,
	Paused,
	End,
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
enum class EHitReactionType : uint8
{
	None,
	Stun,
	Ragdoll,
	Invalid		= 255	UMETA(Hidden)
};

UENUM()
enum class EWeaponType : uint8
{
	Headbutt,
	Trombone,
	Cymbals,
	Violin,
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

UENUM(BlueprintType)
enum class EBarInterpType : uint8
{
	Smooth      UMETA(DisplayName = "Smooth (Ease-Out)"),  // 부드러운 감속 (FInterpTo)
	Constant    UMETA(DisplayName = "Constant (Linear)")   // 일정한 속도 (FInterpConstantTo)
};