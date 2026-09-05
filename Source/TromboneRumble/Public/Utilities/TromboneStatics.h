#pragma once

#include "CoreMinimal.h"
#include "Defines.h"
#include "Data/UIData.h"
#include "DeveloperSettings/TromboneConfig.h"
#include "TromboneGamePlayTags.h"
#include "UI/UserWidgets/Common/RootUI.h"
#include "TromboneStatics.generated.h"

class UFadeWidget;
class ULoadingOverlayWidget;
struct FToastRequest;
class UTwoButtonPopup;
class URootUI;
enum class ELevelType : uint8;

/**
 *  Utility class for static functions in Trombone Rumble Project.
 */
UCLASS()
class TROMBONERUMBLE_API UTromboneStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	
	/** 
	 * Copies the current session's room code to the clipboard.
	 *
	 * @return Whether the room code was successfully copied or not.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "TromboneStatics|Utils", meta = (WorldContext = "WorldContextObject"))
	static bool CopyRoomCodeToClipboard(const UObject* WorldContextObject);
	
	/** Opens a level */
	UFUNCTION(BlueprintCallable, Category = "TromboneStatics|Game", meta = (WorldContext = "WorldContextObject"))
	static void OpenLevel(const UObject* WorldContextObject, ELevelType Level, bool bAbsolute = true);

	/** @return how many players the match menu requires before the host can start.
	 *  Set the console variable "Trombone.MatchMenu.MinPlayersToStart 1" to play alone while testing */
	UFUNCTION(BlueprintPure, Category = "TromboneStatics|Game")
	static int32 GetMinPlayersToStart();

	/** @return true if the given player count reaches GetMinPlayersToStart(). */
	UFUNCTION(BlueprintPure, Category = "TromboneStatics|Game")
	static bool HasEnoughPlayersToStart(int32 PlayerCount);

public:

	/**
	 * Creates FToastRequest with given parameters
	 *
	 * @return Created toast request data structure
	 */
	UFUNCTION(BlueprintPure, Category = "TromboneStatics|UI", meta = (DisplayName = "Make Toast Request", ReturnDisplayName = "Request"))
	static FToastRequest MakeToastRequest(const FText& Message, EToastPosition Position = EToastPosition::BottomCenter, float DisplayDuration = 2.0f);

	/**
	 * Displays a toast message with given FToastRequest
	 *
	 * @return Whether toast was successfully displayed or not
	 */
	UFUNCTION(BlueprintCallable, Category = "TromboneStatics|UI", meta = (WorldContext = "WorldContextObject"))
	static bool ShowToast(const UObject* WorldContextObject, FToastRequest Request);

	/**
	 * Shows a loading overlay
	 * 
	 * @return Added loading overlay widget
	 */
	UFUNCTION(BlueprintCallable, Category = "TromboneStatics|UI")
	static UCommonActivatableWidget* ShowLoadingOverlay(const APlayerController* PlayerController);

	/**
	 * Shows a fade overlay
	 * 
	 * @return Added fade overlay widget
	 */
	UFUNCTION(BlueprintCallable, Category = "TromboneStatics|UI")
	static UFadeWidget* ShowFadeOverlay(const APlayerController* PlayerController);

	/**
	 * Pops the topmost overlay from the stack
	 * 
	 * @return true if overlay was successfully popped, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "TromboneStatics|UI")
	static bool PopOverlay(const APlayerController* PlayerController);
	
public:
	
	/** @return Randomly generated room code of specified length. 
	* If bClipboardCopy is true, the generated code will also be copied to the clipboard. */
	static FString GenerateRandomRoomCode(const int32 CodeLength, const bool bClipboardCopy = true);
	
	/** @return The root UI layout widget
	 *  @see URootUI
	 */
	static URootUI* GetRootUI(const APlayerController* PlayerController);
	
	/**
	 * Show a popup on the Popup layer after loading its class. Input is blocked while the load runs.
	 * NOTE: This operation is async. InitFunc runs before the popup activates.
	 *
	 * @tparam T Popup type. Must have an entry in UTromboneConfig.
	 * @param InitFunc Optional. Put data in the popup before its first frame.
	 */
	template<typename T>
	static void ShowPopupAsync(const UObject* WorldContextObject, TFunction<void(T&)> InitFunc = nullptr);

};

template <typename T>
void UTromboneStatics::ShowPopupAsync(const UObject* WorldContextObject, TFunction<void(T&)> InitFunc)
{
	static_assert(std::is_base_of_v<UCommonActivatableWidget, T>, "T must be a child of UCommonActivatableWidget");

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	URootUI* RootUI = World ? GetRootUI(World->GetFirstPlayerController()) : nullptr;
	if (!RootUI)
	{
		UE_LOG(LogTemp, Error, TEXT("[UTromboneStatics::ShowPopupAsync] RootUI not found"));
		return;
	}

	const TSoftClassPtr<UCommonActivatableWidget> PopupClass = UTromboneConfig::Get()->GetPopupClass<T>();
	if (PopupClass.IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("[UTromboneStatics::ShowPopupAsync] Popup class for type %s not found in config"), *T::StaticClass()->GetName());
		return;
	}

	RootUI->AddWidgetToStackAsync<T>(PopupClass, TromboneGamePlayTags::Trombone_UI_Layer_Popup, true, [InitFunc](const EAsyncPushState State, T* Popup)
	{
		if (State == EAsyncPushState::Initialize && InitFunc && Popup)
		{
			InitFunc(*Popup);
		}
	});
}
