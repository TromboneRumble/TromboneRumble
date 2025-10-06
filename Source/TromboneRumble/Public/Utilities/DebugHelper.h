// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#define CURRENT_CONTEXT FString(TEXT("[")) + FString(__FUNCTION__) + FString(TEXT("] "))

namespace Debug
{
	static void Print(const FString& Msg, int32 InKey = -1, const FColor& InColor = FColor::MakeRandomColor())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(InKey, 7.f, InColor, Msg);

			UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg);
		}
	}

	static void PrintWithCurrentContext(const FString& Msg, int32 InKey = -1, const FColor& InColor = FColor::MakeRandomColor())
	{
		Print(CURRENT_CONTEXT + Msg, InKey, InColor);
	}
}