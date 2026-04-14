#pragma once

#include "CoreMinimal.h"
#include "DeveloperSettings/TromboneConfig.h"
#include "UI/HUD/BaseHUD.h"
#include "UI/UserWidgets/Common/BaseUIRoot.h"
#include "TromboneStatics.generated.h"

class UNoticePopupWidget;
class UTwoButtonWithoutClosePopup;
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
	
	UCommonActivatableWidget* Popup = RootUI->PushPopup(PopupClass);
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
