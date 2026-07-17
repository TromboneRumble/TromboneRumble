// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "CommonUserWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "BaseUIRoot.generated.h"

enum class EUIStackType;
enum class ECommonInputType : uint8;

UCLASS()
class TROMBONERUMBLE_API UBaseUIRoot : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	
	/** Pushes a new widget of the specified class onto the given UI stack.
	 * @param WidgetClass The class of the widget to create.
	 * @param StackType The target UI stack where the widget will be added.
	 * @return Created widget instance.
	 */
	UCommonActivatableWidget* AddWidgetToStack(const TSubclassOf<UCommonActivatableWidget> WidgetClass, const EUIStackType StackType) const;

	/** Pushes a new widget of the specified class onto the given UI stack and initializes it.
	 * @param WidgetClass same as above
	 * @param StackType same as above
	 * @param InstanceInitFunc A callback function to initialize the widget instance.
	 * @return Created widget instance.
	 */
	template <typename T>
	T* AddWidgetToStack(const TSubclassOf<UCommonActivatableWidget> WidgetClass, const EUIStackType StackType, TFunctionRef<void(T&)> InstanceInitFunc) const;

	/** Pops the top widget from the specified UI stack.
	 * @param StackType same as above
	 * @return True if a widget was successfully removed, false otherwise.
	 */
	bool PopStack(const EUIStackType StackType) const;
	
	/** Enabling/Disabling the base UI */
	void SetBaseUIEnabled(const bool bEnabled) const;

	/** @return The widget that currently owns UI focus priority (topmost popup, else active base screen). */
	UCommonActivatableWidget* GetTopActiveWidget() const;

	/** Focuses the desired focus target of the top active widget. Used to (re)seed gamepad focus. */
	void FocusActiveWidgetDesiredTarget() const;

protected:

	UCommonActivatableWidgetStack* GetStackByType(const EUIStackType StackType) const;

private:

	/** Handles mouse <-> gamepad switching: seeds focus and toggles the cursor for gamepad play. */
	void HandleInputMethodChanged(ECommonInputType NewInputType);

	/** True while the cursor is hidden because of gamepad input, so we only restore what we hid. */
	bool bCursorHiddenForGamepad = false;

protected:
	
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> BaseStack;
	
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> PopupStack;
	
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> OverlayStack;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCommonActivatableWidget> DefaultWidgetClass;

protected:
	
	// ~ Begin UCommonUserWidget Interface
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	// ~ End UCommonUserWidget Interface
};

template <typename T>
T* UBaseUIRoot::AddWidgetToStack(const TSubclassOf<UCommonActivatableWidget> WidgetClass, const EUIStackType StackType, TFunctionRef<void(T&)> InstanceInitFunc) const
{
	if (!WidgetClass) return nullptr;

	UCommonActivatableWidgetStack* TargetStack = GetStackByType(StackType);
	if (!TargetStack) return nullptr;

	if (const UCommonActivatableWidget* ActiveWidget = TargetStack->GetActiveWidget())
	{
		if (ActiveWidget->GetClass() == WidgetClass)
		{
			return Cast<T>(TargetStack->GetActiveWidget());
		}
	}

	return TargetStack->AddWidget<T>(WidgetClass, InstanceInitFunc);
}