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
	Drunkard,
	BeerFlood,
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
	Customize,

	OrchestraStageLobby,
	SnowFieldLobby,
	JazzBarLobby,

	OrchestraStage,
	SnowField,
	JazzBar,

	Result,

	Invalid = 255 UMETA(Hidden)
};

/** @return true if the given level type is result level */
FORCEINLINE bool IsResultLevelType(const ELevelType Type)
{
	return Type == ELevelType::Result;
}

/** @return true if the given level type is in-game level */
FORCEINLINE bool IsInGameLevelType(const ELevelType Type)
{
	return Type == ELevelType::OrchestraStage || Type == ELevelType::SnowField || Type == ELevelType::JazzBar;
}

/** @return true if the given level type is lobby level */
FORCEINLINE bool IsLobbyLevelType(const ELevelType Type)
{
	return Type == ELevelType::OrchestraStageLobby || Type == ELevelType::SnowFieldLobby || Type == ELevelType::JazzBarLobby;
}

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
	Drunkard,
};

UENUM()
enum class EHitReactionType : uint8
{
	None,
	Stun,
	Ragdoll,
	KnockbackOnly,
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

	// 원형 윈도우가 "가리는 물체가 그려진 픽셀"만 골라내는 데 쓰는 값 (UXRayWindowComponent 전용)
	constexpr int32 OCCLUDER_STENCIL = 251;

	constexpr float MAX_FRAME_RATE = 144.f;
}

namespace TromboneMaterial
{
	// 머티리얼 슬롯/파라미터 이름
	inline const FName SkinSlotName(TEXT("skin"));
	inline const FName FaceSlotName(TEXT("face"));
	inline const FName BaseColorParam(TEXT("BaseColor"));

	// X-Ray 실루엣 (M_XRaySilhouette의 파라미터, UXRaySilhouetteComponent가 구동)
	inline const FName SilhouetteColorParam(TEXT("SilhouetteColor"));

	// Wall Occlusion 디더 페이드 (MF_OcclusionFade의 ScalarParameter, UXRayTranslucentFadeComponent가 구동)
	inline const FName OcclusionFadeParam(TEXT("OcclusionFade"));
	inline const FName FadedOpacityParam(TEXT("FadedOpacity"));

	// X-Ray 원형 윈도우 (M_PP_OcclusionWindow의 파라미터, UXRayWindowComponent가 구동)
	inline const FName OcclusionCaptureRTParam(TEXT("OcclusionCaptureRT"));
	inline const FName PlayerScreenUVParam(TEXT("PlayerScreenUV"));	// xy=원 중심 UV, z=화면 종횡비
	inline const FName HoleScreenRadiusParam(TEXT("HoleScreenRadius"));	// 뷰포트 높이로 정규화된 반경
	inline const FName WindowFadeParam(TEXT("WindowFade"));
	inline const FName EdgeSoftnessParam(TEXT("EdgeSoftness"));	// 원 가장자리 그라데이션 폭 (0=칼같은 경계)
	inline const FName WallDimParam(TEXT("WallDim"));	// 원 안쪽 벽을 얼마나 어둡게 할지 (1=그대로)
	// 캡처가 담고 있는 화면 영역. 둘 다 xy만 쓴다 — VectorParameter의 기본 출력이 float3라 zw를 못 뽑는다.
	// min (0,0) + size (1,1)이면 화면 전체
	inline const FName CaptureRectMinParam(TEXT("CaptureRectMin"));
	inline const FName CaptureRectSizeParam(TEXT("CaptureRectSize"));
}

namespace TromboneBones
{
	const FName Pelvis = TEXT("pelvis");
	const FName Flage = TEXT("flage01");
}