// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#define PRINT_WITH_CURRENT_CONTEXT(Msg) Debug::Print(FString::Printf(TEXT("[%s] %s"), *FString(__FUNCTION__), *FString(Msg)))

namespace Debug
{
	static void Print(const FString& Msg, int32 InKey = -1, const FColor& InColor = FColor::MakeRandomColor())
	{
#if WITH_EDITOR
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(InKey, 7.f, InColor, Msg);

			UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg);
		}
#endif
	}
}