// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "RhythmScoreAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class TROMBONERUMBLE_API URhythmScoreAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	URhythmScoreAttributeSet();

	UPROPERTY(BlueprintReadOnly, Category = "Score")
	FGameplayAttributeData GradeMultiplier;
	ATTRIBUTE_ACCESSORS(URhythmScoreAttributeSet, GradeMultiplier);

	UPROPERTY(BlueprintReadOnly, Category = "Score")
	FGameplayAttributeData ComboMultiplier;
	ATTRIBUTE_ACCESSORS(URhythmScoreAttributeSet, ComboMultiplier);
};
