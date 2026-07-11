// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Defines.generated.h"

UENUM()
enum class EUIStackType
{
	Base,
	Popup,
	Overlay
};

UENUM()
enum class EMatchType : uint8
{
	Public = 0,
	Custom = 1,
	MAX,
	None = 255,
};

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
	Present,
	PressurePlate,
	Ice,
	Blizzard,
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
	Cry,
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
	Cry1 = 8,
	Cry2 = 9,
	Cry3 = 10,
	None = 255,
};

UENUM()
enum class EEquipmentSlotType : uint8
{
	Weapon,
	MAX_SLOTS
};

UENUM()
enum class ELevelType : uint8
{
	MainMenu,
	MatchMenu,
	Tutorial,
	ResultScene,
	Customize,
	
	OrchestraStageLobby,
	SnowFieldLobby,
	
	OrchestraStage,
	SnowField,
	
	Invalid = 255 UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ECustomizationSlotType : uint8
{
	Antenna UMETA(DisplayName = "Antenna"),
	Face    UMETA(DisplayName = "Face"),
	Costume UMETA(DisplayName = "Costume"),
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
	None,
	WaitingForPlayers,
	FallingPlayers,
	CountdownToStandup,
	InstrumentScramble,
	CountdownToTravel,
};

UENUM()
enum class EHitInstigatorType : uint8
{
	None,
	Headbutt,
	Trombone,
	Cymbals,
	Violin,
	Garbage_Cup,
	Garbage_Paper,
	Garbage_Chair,
	PressurePlate,
	Blizzard,
};

UENUM()
enum class EHitReactionType : uint8
{
	None,
	Stun,
	Ragdoll,
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
enum class ERhythmGameState : uint8
{
	None = 0		UMETA(DisplayName = "None"), 
	Start = 1		UMETA(DisplayName = "Start"),  
	Playing = 2		UMETA(DisplayName = "Playing"),
	Paused = 3		UMETA(DisplayName = "Paused"),
	Resumed = 4		UMETA(DisplayName = "Resumed"),
	Stopped = 5		UMETA(DisplayName = "Stopped"),
	Ended			UMETA(DisplayName = "Ended"),
	Invalid = 255   UMETA(Hidden)
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

UENUM(BlueprintType)
enum class EScoreType : uint8
{
	RhythmScore = 0				UMETA(DisplayName = "Rhythm"),
	BuffedTromboneScore = 1		UMETA(DisplayName = "BuffedTromboneScore"),
	BuffedViolinScore = 2		UMETA(DisplayName = "BuffedViolinScore"),
	InstrumentPickedUp = 3		UMETA(DisplayName = "InstrumentPickedUp"),
	OnHit = 4					UMETA(DisplayName = "OnHit"),
	CymbalsHit = 5				UMETA(DisplayName = "CymbalsHit"),
	SpotLight = 6				UMETA(DisplayName = "SpotLight"),
	Present = 7					UMETA(DisplayName = "Present"),

	None = 254					UMETA(DisplayName = "None"),
	Invalid = 255				UMETA(Hidden)
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

UENUM(BlueprintType)
enum class EVoipMode : uint8
{
	None       = 0 UMETA(DisplayName = "None"),        // 음소거: 전송 없음, PTT 키 무시
	PushToTalk = 1 UMETA(DisplayName = "Push To Talk"),
	AutoVoice  = 2 UMETA(DisplayName = "Auto Voice"),
};

namespace TromboneRender
{
	// PostProcess X-Ray 머티리얼이 CustomStencil == 이 값일 때 가려진 실루엣 렌더
	constexpr int32 CHARACTER_OCCLUDED_STENCIL = 252;
}

namespace TromboneMaterial
{
	// 머티리얼 슬롯/파라미터 이름
	inline const FName SkinSlotName(TEXT("skin"));
	inline const FName FaceSlotName(TEXT("face"));
	inline const FName BaseColorParam(TEXT("BaseColor"));
}

namespace TromboneBones
{
	const FName Pelvis = TEXT("pelvis");
	const FName Flage = TEXT("flage01");
}