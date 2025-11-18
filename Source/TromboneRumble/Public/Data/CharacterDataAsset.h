// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CharacterDataAsset.generated.h"

UCLASS()
class TROMBONERUMBLE_API UCharacterDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Config|Movement|Ground", meta = (Tooltip = "캐릭터의 기본 걷기 속도입니다. (cm/s)"))
    float WalkSpeed = 300.0f;

    UPROPERTY(EditAnywhere, Category = "Config|Movement|Ground", meta = (Tooltip = "캐릭터의 전력 질주 속도입니다. (cm/s)"))
    float SprintSpeed = 500.0f;

    UPROPERTY(EditAnywhere, Category = "Config|Movement|Ground", meta = (Tooltip = "캐릭터가 회전하는 속도입니다. (degrees/s)"))
    float RotationRate = 360.f;
    
    UPROPERTY(EditAnywhere, Category = "Config|Movement|Air", meta = (Tooltip = "점프 시 수직으로 솟아오르는 초기 속도입니다. 높을수록 높이 점프합니다. (cm/s)"))
    float JumpZVelocity = 500.f;

    UPROPERTY(EditAnywhere, Category = "Config|Movement|Air", meta = (ClampMin = "0.0", ClampMax = "1.0", Tooltip = "공중에서 플레이어가 캐릭터를 제어할 수 있는 정도입니다. (0.0 = 제어 불가, 1.0 = 즉시 반응)"))
    float AirControl = 0.35f;

    UPROPERTY(EditAnywhere, Category = "Config|Movement|Air", meta = (Tooltip = "캐릭터에게 적용되는 중력의 배율입니다. 높을수록 빨리 떨어집니다."))
    float GravityScale = 1.5f;
    
    UPROPERTY(EditAnywhere, Category = "Config|Movement|Inertia", meta = (Tooltip = "캐릭터가 목표 속도에 도달하기까지의 가속도입니다. 높을수록 반응이 빠릅니다."))
    float MaxAcceleration = 1000.0f;

    UPROPERTY(EditAnywhere, Category = "Config|Movement|Inertia", meta = (Tooltip = "(걷기 중) 플레이어가 입력을 멈췄을 때 감속하는 속도입니다. 높을수록 '미끄러지지 않고' 즉시 멈춥니다."))
    float BrakingDecelerationWalking = 400.f;

    UPROPERTY(EditDefaultsOnly, Category = "Config|Movement|Inertia", meta = (Tooltip = "지면과의 마찰력입니다. 높을수록 경사면 등에서 덜 미끄러집니다."))
    float GroundFriction = 3.0f;
    
    UPROPERTY(EditAnywhere, Category = "Config|Reactions", meta = (Tooltip = "래그돌 상태가 지속되는 시간입니다. (초)"))
    float RagdollDuration = 2.5f;
    
    UPROPERTY(EditAnywhere, Category = "Config|Reactions", meta = (Tooltip = "스턴 상태가 지속되는 시간입니다. (초)"))
    float StunDuration = 1.5f;

    UPROPERTY(EditAnywhere, Category = "Config|Camera", meta = (Tooltip = "카메라와 캐릭터 사이의 기본 거리입니다."))
    float TargetArmLength = 650.0f;
    
    UPROPERTY(EditAnywhere, Category = "Config|Camera", meta = (Tooltip = "카메라가 캐릭터를 내려다보는 각도입니다."))
    float CameraRelativeRotationPitch = -30.0f;
    
    UPROPERTY(EditAnywhere, Category = "Config|Camera", meta = (Tooltip = "카메라 붐(스프링 암)이 부착되는 높이입니다."))
    float CameraRelativeLocationZ = 60.0f;
};