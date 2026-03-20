#pragma once

#include "CoreMinimal.h"
#include "TromboneStatics.generated.h"

class UBaseUIRoot;
enum class ELevelState : uint8;

/**
 *  Utility class for static functions in Trombone Rumble Project.
 */
UCLASS()
class TROMBONERUMBLE_API UTromboneStatics : public UObject
{
	GENERATED_BODY()
	
public:
	
	/** Opens a level */
	static void OpenLevel(const UObject* WorldContextObject, ELevelState Level, bool bAbsolute = true);
	
	/** Configures player input */
	static void SetInputConfig(const UObject* WorldContextObject, bool bFocusUI, bool bShowCursor, bool bIgnoreInput = false, bool bRemoveMappingContext = false);	
	
	/** @return The root UI layout widget
	 *  @see UBaseUIRoot
	 */
	static UBaseUIRoot* GetRootLayout(const APlayerController* PlayerController);
};
