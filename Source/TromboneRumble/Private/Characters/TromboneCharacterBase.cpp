// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/TromboneCharacterBase.h"

ATromboneCharacterBase::ATromboneCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

}

void ATromboneCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void ATromboneCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATromboneCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

