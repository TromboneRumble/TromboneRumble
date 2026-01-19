// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/Rhythm/ComboWidget/RhythmComboWidgetBase.h"

#include "Components/TextBlock.h"
#include "Components/ActorComponents/EquipmentComponent.h"
#include "Framework/DefaultPlayerState.h"
#include "Subsystems/RhythmSubsystem.h"
#include "Utilities/Defines.h"

void URhythmComboWidgetBase::Init(AInstrumentBase* InOwner)
{
	OwnerInstrument = InOwner;
}