// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CharacterDataAsset.generated.h"

enum class ECharacterFaceState : uint8;
enum class ECharacterFaceType : uint8;

USTRUCT(BlueprintType)
struct FCharacterFaceAnimationSequence
{
	GENERATED_BODY()

	/** 표정 시퀀스 배열 */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "표정 시퀀스"))
	TArray<ECharacterFaceType> Sequence;

	/** 각 표정이 지속되는 간격 (초) */
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.05", ClampMax = "3.0", DisplayName = "표정 간격"))
	float Interval = 0.1f;

	/** 시퀀스 반복 여부 */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "반복 여부"))
	bool bLoop = false;
	
	/** 루프 시 다음 반복까지의 최소 지연 시간 (초) */
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bLoop", ClampMin = "0.0", ClampMax = "5.0", DisplayName = "최소 루프 지연 시간"))
	float MinLoopDelay = 0.0f;
	
	/** 루프 시 다음 반복까지의 최대 지연 시간 (초) */
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bLoop", ClampMin = "0.0", ClampMax = "5.0", DisplayName = "최대 루프 지연 시간"))
	float MaxLoopDelay = 0.0f;
};

USTRUCT(BlueprintType)
struct FRageBuffMileStone
{
    GENERATED_BODY()

    /** 이 시간에 도달하면 배율 적용 (초) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeThreshold = 0.0f;

    /** 적용할 이동 속도 배율 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpeedMultiplier = 1.0f;
};

/*
 * DataAsset 형태로 캐릭터 초기 설정값 보관
 * 인게임에서 변경될 값(Health, Stamina 등)은 CharacterAttributeSet에서 관리
 */
UCLASS()
class TROMBONERUMBLE_API UCharacterDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
    /** 캐릭터의 기본 걷기 속도입니다. (cm/s) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Movement|Ground", meta = (DisplayName = "기본 이동 속도"))
    float WalkSpeed = 300.0f;

    /** 캐릭터의 전력 질주 속도입니다. (cm/s) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Movement|Ground", meta = (DisplayName = "전력 질주 속도"))
    float SprintSpeed = 500.0f;
    
    /** 아이템 장착 시 이동 속도 배율입니다. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Movement|Ground", meta = (DisplayName = "아이템 장착 시 이동 속도 배율"))
    float EquippedMovementSpeedMultiplier = 0.8f;

    /** 캐릭터가 회전하는 속도입니다. (degrees/s) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Movement|Ground", meta = (DisplayName = "회전 속도"))
    float RotationRate = 360.f;
    
    /** 점프 시 수직으로 솟아오르는 초기 속도입니다. 높을수록 높이 점프합니다. (cm/s) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Movement|Air", meta = (DisplayName = "점프 수직 속도"))
    float JumpZVelocity = 500.f;

    /** 공중에서 플레이어가 캐릭터를 제어할 수 있는 정도입니다. (0.0 = 제어 불가, 1.0 = 즉시 반응) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Movement|Air", meta = (ClampMin = "0.0", ClampMax = "1.0", DisplayName = "공중 제어력"))
    float AirControl = 0.35f;

    /** 캐릭터에게 적용되는 중력의 배율입니다. 높을수록 빨리 떨어집니다. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Movement|Air", meta = (DisplayName = "중력 배율"))
    float GravityScale = 1.5f;
    
    /** 캐릭터가 목표 속도에 도달하기까지의 가속도입니다. 높을수록 반응이 빠릅니다. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Movement|Inertia", meta = (DisplayName = "최대 가속도"))
    float MaxAcceleration = 1000.0f;

    /** (걷기 중) 플레이어가 입력을 멈췄을 때 감속하는 속도입니다. 높을수록 '미끄러지지 않고' 즉시 멈춥니다. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Movement|Inertia", meta = (DisplayName = "걷기 감속도"))
    float BrakingDecelerationWalking = 400.f;

    /** 지면과의 마찰력입니다. 높을수록 경사면 등에서 덜 미끄러집니다. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Movement|Inertia", meta = (DisplayName = "지면 마찰력"))
    float GroundFriction = 3.0f;
    
    /** 래그돌 상태가 지속되는 시간입니다. (초) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Reactions", meta = (DisplayName = "래그돌 지속 시간"))
    float RagdollDuration = 2.5f;
    
    /** 스턴 상태가 지속되는 시간입니다. (초) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Reactions", meta = (DisplayName = "스턴 지속 시간"))
    float StunDuration = 1.5f;
	
    /** 래그돌 후 무적 시간입니다. 래그돌에서 일어나는 애니메이션 재생 시간 2초를 포함합니다. (초) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Reactions", meta = (DisplayName = "래그돌 후 무적 시간"))
	float InvincibilityDurationAfterRagdoll = 3.0f;
	
	/** 스턴 후 무적 시간입니다. (초) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Reactions", meta = (DisplayName = "스턴 후 무적 시간"))
	float InvincibilityDurationAfterStun = 0.5f;

    /** 카메라와 캐릭터 사이의 기본 거리입니다. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Camera", meta = (DisplayName = "카메라 거리"))
    float TargetArmLength = 650.0f;
    
    /** 카메라가 캐릭터를 내려다보는 각도입니다. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Camera", meta = (DisplayName = "카메라 각도 (Pitch)"))
    float CameraRelativeRotationPitch = -30.0f;
    
    /** 카메라 붐(스프링 암)이 부착되는 높이입니다. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Camera", meta = (DisplayName = "카메라 부착 높이"))
    float CameraRelativeLocationZ = 60.0f;

    /** 카메라 줌 레벨 1 (가까이) Arm Length */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Camera|Zoom", meta = (DisplayName = "줌 레벨 1 거리"))
    float CameraArmLengthLevel1 = 400.0f;

    /** 카메라 줌 레벨 2 (기본) Arm Length */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Camera|Zoom", meta = (DisplayName = "줌 레벨 2 거리"))
    float CameraArmLengthLevel2 = 800.0f;

    /** 카메라 줌 레벨 3 (멀리) Arm Length */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Camera|Zoom", meta = (DisplayName = "줌 레벨 3 거리"))
    float CameraArmLengthLevel3 = 1500.0f;

    /** 카메라 줌 레벨 1 Pitch (도) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Camera|Zoom", meta = (DisplayName = "줌 레벨 1 Pitch"))
    float CameraPitchLevel1 = -20.0f;

    /** 카메라 줌 레벨 2 Pitch (도) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Camera|Zoom", meta = (DisplayName = "줌 레벨 2 Pitch"))
    float CameraPitchLevel2 = -30.0f;

    /** 카메라 줌 레벨 3 Pitch (도) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Camera|Zoom", meta = (DisplayName = "줌 레벨 3 Pitch"))
    float CameraPitchLevel3 = -35.0f;

    /** 줌 레벨 변경 시 보간 속도 (FInterpTo/RInterpTo InterpSpeed) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Camera|Zoom", meta = (DisplayName = "줌 보간 속도"))
    float CameraZoomInterpSpeed = 8.0f;

    /** 줌 시 벽과의 안전 여유 거리(cm) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Camera|Zoom", meta = (DisplayName = "줌 벽 여유 거리"))
    float CameraZoomCollisionMargin = 10.0f;

	/** 얼굴 표정 애니메이션 시퀀스 맵 */
	UPROPERTY(EditAnywhere, Category = "Config|Facial Expressions", meta = (DisplayName = "얼굴 표정 애니메이션 시퀀스"))
	TMap<ECharacterFaceState, FCharacterFaceAnimationSequence> FaceSequences;

    /** 장비를 장착하지 않았을 때 시간에 따라 강화되는 분노 버프 설정 */
    UPROPERTY(EditAnywhere, Category = "Config|Movement|Buff", meta = (DisplayName = "분노 버프 마일스톤"))
    TArray<FRageBuffMileStone> SpeedBuffMilestones;

    float GetMaxBuffTime() const;
};