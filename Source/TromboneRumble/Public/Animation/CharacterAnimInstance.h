// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CharacterAnimInstance.generated.h"

UCLASS()
class TROMBONERUMBLE_API UCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void SetIsAttacking(const bool bNewIsAttacking);
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Animation")	
	float UpperBodyBlendAlpha = 0.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsAttacking = false;
	
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float BlendInterpSpeed = 15.0f;
};
