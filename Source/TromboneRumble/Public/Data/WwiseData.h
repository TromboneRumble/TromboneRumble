#pragma once

#include "CoreMinimal.h"

namespace WwiseRTPC
{
	const FName MasterVolume = FName(TEXT("RTPC_MasterVolume"));
	const FName BGMVolume    = FName(TEXT("RTPC_BGMVolume"));
	const FName MusicVolume  = FName(TEXT("RTPC_MusicVolume"));
	const FName SFXVolume    = FName(TEXT("RTPC_SFXVolume"));
	
	void SetVolume(FName RTPCName, float Value);
}