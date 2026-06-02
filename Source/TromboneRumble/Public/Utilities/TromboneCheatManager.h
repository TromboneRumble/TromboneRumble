#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "TromboneCheatManager.generated.h"

enum class EWeaponType : uint8;

/* Press '~' in-game to open the console and type commands below
 */
UCLASS()
class TROMBONERUMBLE_API UTromboneCheatManager : public UCheatManager
{
	GENERATED_BODY()
	
/** Gameplay Console Commands */
public: 
	
	UFUNCTION(Exec)
	void Trombone_Help();
	
	UFUNCTION(Exec)
	void Trombone_SpawnInstrument(const FString& TypeString);
	
	UFUNCTION(Exec)
	void Trombone_Spotlight();
	
	UFUNCTION(Exec)
	void Trombone_Throw(const FString& Count);

	UFUNCTION(Exec)
	void Trombone_Ragdoll();

	UFUNCTION(Exec)
	void Trombone_Stun();
	
	UFUNCTION(Exec)
	void Trombone_ResetSettingData();
	
	
public:
	
	UFUNCTION(Exec)
	void Trombone_Dump_LevelStateSubsystem();
};
