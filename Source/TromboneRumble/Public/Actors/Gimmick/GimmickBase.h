// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utilities/Defines.h"
#include "GimmickBase.generated.h"

UCLASS()
class TROMBONERUMBLE_API AGimmickBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AGimmickBase();
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UFUNCTION()
	virtual void Activate();
	
	UFUNCTION()
	virtual	void Deactivate();
	
	bool IsActive() const { return bIsActive; }
	EGimmickType GetGimmickType() const { return GimmickType; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Config")
	EGimmickType GimmickType = EGimmickType::None;

	UPROPERTY(VisibleAnywhere)
	bool bIsActive = false;
};