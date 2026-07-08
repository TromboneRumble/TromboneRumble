#pragma once

#include "CoreMinimal.h"
#include "Defines.h"
#include "Data/UIData.h"
#include "DeveloperSettings/TromboneConfig.h"
#include "UI/HUD/BaseHUD.h"
#include "UI/UserWidgets/Common/BaseUIRoot.h"
#include "TromboneStatics.generated.h"

class UFadeWidget;
class ULoadingOverlayWidget;
struct FToastRequest;
class UNoticePopup;
class UTwoButtonPopup;
class UBaseUIRoot;
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
	 *  @see UBaseUIRoot
	 */
	static UBaseUIRoot* GetRootLayout(const APlayerController* PlayerController);
	
	/** Shows a popup
	 * @tparam T The type of the popup widget. Must be a child of UCommonActivatableWidget and have a corresponding entry in UTromboneConfig.
	 * @return popup widget instance of type T
	 */
	template<typename T>
	static T* ShowPopup(const UObject* WorldContextObject);
	
};

template <typename T>
T* UTromboneStatics::ShowPopup(const UObject* WorldContextObject)
{
	static_assert(std::is_base_of_v<UCommonActivatableWidget, T>, "T must be a child of UCommonActivatableWidget");
	
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

	const UTromboneConfig* Config = UTromboneConfig::Get();
	
	AHUD* Hud = World->GetFirstPlayerController()->GetHUD();
	const ABaseHUD* BaseHud = Cast<ABaseHUD>(Hud);
	if (!BaseHud)
	{
		UE_LOG(LogTemp, Error, TEXT("[UTromboneStatics::ShowPopup] BaseHUD not found"));
		return nullptr;
	}
	
	UBaseUIRoot* RootUI = BaseHud->GetRootUI();
	if (!RootUI)
	{
		UE_LOG(LogTemp, Error, TEXT("[UTromboneStatics::ShowPopup] RootUI not found in BaseHUD"));
		return nullptr;
	}
	
	TSubclassOf<T> PopupClass = Config->GetPopupClass<T>();
	if (!PopupClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[UTromboneStatics::ShowPopup] Popup class for type %s not found in config"), *T::StaticClass()->GetName());
		return nullptr;
	}
	
	if (!PopupClass->IsChildOf(UCommonActivatableWidget::StaticClass()))
	{
		UE_LOG(LogTemp, Error, TEXT("[UTromboneStatics::ShowPopup] Popup class %s is not a child of %s"), *PopupClass->GetName(), *UCommonActivatableWidget::StaticClass()->GetName());
		return nullptr;
	}
	
	UCommonActivatableWidget* Popup = RootUI->AddWidgetToStack(PopupClass, EUIStackType::Popup);
	if (!Popup)
	{
		UE_LOG(LogTemp, Error, TEXT("[UTromboneStatics::ShowPopup] Failed to push popup of type %s to RootUI"), *T::StaticClass()->GetName());
		return nullptr;
	}
	
	T* TypedPopup = Cast<T>(Popup);
	if (!TypedPopup)
	{
		UE_LOG(LogTemp, Error, TEXT("[UTromboneStatics::ShowPopup] Failed to cast popup to type %s"), *T::StaticClass()->GetName());
		return nullptr;
	}
	
	return TypedPopup;
}
