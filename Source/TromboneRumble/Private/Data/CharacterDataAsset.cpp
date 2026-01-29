// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/CharacterDataAsset.h"

float UCharacterDataAsset::GetMaxBuffTime() const
{
    float MaxTime = 0.0f;
    for (const auto& Milestone : SpeedBuffMilestones)
    {
        MaxTime = FMath::Max(MaxTime, Milestone.TimeThreshold);
    }
    return MaxTime > 0.0f ? MaxTime : 1.0f;
}
