// Copyright (C) 2026 biksari studio. All Rights Reserved.

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
	
	/** Turns the gimmick on. */
	UFUNCTION(BlueprintCallable, Category = "Gimmick")
	virtual void Activate();
	
	/** Turns the gimmick off. */
	UFUNCTION(BlueprintCallable, Category = "Gimmick")
	virtual void Deactivate();
	
	bool IsActive() const { return bIsActive; }
	EGimmickType GetGimmickType() const { return GimmickType; }
	
protected:
	
	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Config")
	EGimmickType GimmickType = EGimmickType::None;
	
	UPROPERTY(VisibleAnywhere)
	bool bIsActive = false;
	
protected:
	
	//~ Begin AActor Interface
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor Interface
	
};
