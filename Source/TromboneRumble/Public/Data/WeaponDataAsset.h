// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Utilities/Defines.h"
#include "WeaponDataAsset.generated.h"

UCLASS()
class TROMBONERUMBLE_API UWeaponDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 무기 공격 후 다음 공격까지 걸리는 시간 (초) */
	UPROPERTY(EditAnywhere, Category = "Config", meta = (DisplayName = "공격 재사용 대기시간"))
	float AttackCooldown = 0.0f;
    
	/** 공격 적중 시 상대방을 수평으로 밀어내는 힘의 크기 */
	UPROPERTY(EditAnywhere, Category = "Config", meta = (DisplayName = "넉백 힘 (수평)"))
	float KnockbackForce = 500.0f;

	/** 공격 적중 시 상대방을 위로 띄우는 힘의 크기 */
	UPROPERTY(EditAnywhere, Category = "Config", meta = (DisplayName = "넉백 힘 (수직)"))
	float KnockbackUpForce = 300.0f;
    
	/** 무기를 떨어뜨릴 때 전방으로 가해지는 충격량 */
	UPROPERTY(EditAnywhere, Category = "Config", meta = (DisplayName = "무기 드롭 전방 충격량"))
	float WeaponDropForwardImpulse = 500.0f;
    
	/** 무기를 떨어뜨릴 때 위쪽으로 가해지는 충격량 */
	UPROPERTY(EditAnywhere, Category = "Config", meta = (DisplayName = "무기 드롭 상향 충격량"))
	float WeaponDropUpwardImpulse = 300.0f;
    
	/** 무기 공격 적중 시 상대방에게 적용할 피격 유형 */
	UPROPERTY(EditDefaultsOnly, Category = "Config", meta = (DisplayName = "피해 유형"))
	EHitReactionType HitReactionType = EHitReactionType::None;
    
	UPROPERTY(EditDefaultsOnly, Category = "Develop")
	EWeaponType WeaponType = EWeaponType::Invalid;
    
	UPROPERTY(EditDefaultsOnly, Category = "Develop")
	FName EquipSocketName = TEXT("socket_hand_l");
};