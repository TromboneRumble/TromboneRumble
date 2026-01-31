// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Tutorial/TutorialDummy.h"

ATutorialDummy::ATutorialDummy()
{
	PrimaryActorTick.bCanEverTick = true;

}

void ATutorialDummy::BeginPlay()
{
	Super::BeginPlay();
	
}

void ATutorialDummy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}