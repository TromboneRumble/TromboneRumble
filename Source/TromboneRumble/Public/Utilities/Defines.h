#pragma once

#include "CoreMinimal.h"
#include "Defines.generated.h"


// TODO : 추후 팀전 고려.
UENUM()
enum class EMatchState : uint8
{
	FreeForAll,
	TwoTeams,
	Invalid UMETA(Hidden)
};
