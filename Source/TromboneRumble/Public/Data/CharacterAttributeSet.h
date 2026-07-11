// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "CharacterAttributeSet.generated.h"


#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class TROMBONERUMBLE_API UCharacterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
    UCharacterAttributeSet() {}
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
    virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

    UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_MoveSpeed)
    FGameplayAttributeData MoveSpeed;
    
    // 얼음 미끄러짐용 지면 마찰
    UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_GroundFriction)
    FGameplayAttributeData GroundFriction;
    
    // 얼음 미끄러짐용 제동 감속
    UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_BrakingDeceleration)
    FGameplayAttributeData BrakingDeceleration;
    
    // 얼음 위 걷기 애니메이션 배속
    UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_LocomotionPlayRate)
    FGameplayAttributeData LocomotionPlayRate;

protected:
    UFUNCTION()
    void OnRep_MoveSpeed(const FGameplayAttributeData& OldValue);

    UFUNCTION()
    void OnRep_GroundFriction(const FGameplayAttributeData& OldValue);

    UFUNCTION()
    void OnRep_BrakingDeceleration(const FGameplayAttributeData& OldValue);

    UFUNCTION()
    void OnRep_LocomotionPlayRate(const FGameplayAttributeData& OldValue);

public:
    ATTRIBUTE_ACCESSORS(UCharacterAttributeSet, MoveSpeed)
    ATTRIBUTE_ACCESSORS(UCharacterAttributeSet, GroundFriction)
    ATTRIBUTE_ACCESSORS(UCharacterAttributeSet, BrakingDeceleration)
    ATTRIBUTE_ACCESSORS(UCharacterAttributeSet, LocomotionPlayRate)
};
