// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "CommonUserWidget.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "GameplayTagContainer.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "RootUI.generated.h"

enum class ECommonInputType : uint8;
class UCommonActionWidget;
class UPerformanceWidget;
class UProjectVersionWidget;

/** Where an async push currently is when its callback runs. */
enum class EAsyncPushState : uint8
{
	/** The load was canceled. The widget pointer is null. */
	Canceled,

	/** The widget exists but is not active yet. Put data in here so the first frame is not empty. */
	Initialize,

	/** The widget is on its layer. */
	AfterPush
};

/**
 * RootUI hosts the UI layers of one local player and the overlays that live as long as the root.
 * It is created once by UTromboneUISubsystem and attached to the player controller of each level.
 * Layers are looked up by tag. The three default stacks register themselves, extra layers call RegisterLayer from the widget blueprint.
 * Keep in mind that layers are ordered by registration, bottom first, and focus priority follows that order.
 *
 * @see UTromboneUISubsystem
 * @see ATromboneHUD
 */
UCLASS()
class TROMBONERUMBLE_API URootUI : public UCommonUserWidget
{
	GENERATED_BODY()

public:

	/**
	 * Push a widget of the given class onto a layer. If the top of the layer is already this class, that instance is returned.
	 *
	 * @param WidgetClass Class to create.
	 * @param LayerTag Layer to push onto. Check out TromboneGamePlayTags for the built in layers.
	 * @return The widget on top of the layer, or null when the class or layer is invalid.
	 */
	UCommonActivatableWidget* AddWidgetToStack(TSubclassOf<UCommonActivatableWidget> WidgetClass, FGameplayTag LayerTag) const;

	/**
	 * Same as above, but runs InstanceInitFunc on the widget before it activates.
	 *
	 * @param WidgetClass Class to create.
	 * @param LayerTag Layer to push onto.
	 * @param InstanceInitFunc Called with the new instance before activation.
	 * @return The widget on top of the layer, or null when the class or layer is invalid.
	 */
	template <typename T>
	T* AddWidgetToStack(TSubclassOf<UCommonActivatableWidget> WidgetClass, FGameplayTag LayerTag, TFunctionRef<void(T&)> InstanceInitFunc) const;

	/**
	 * Load the class first, then push it. Input is blocked while the load runs so the player cannot act on a half built screen.
	 * NOTE: This operation is async. StateFunc runs once for Initialize and once for AfterPush, or once for Canceled.
	 *
	 * @param WidgetClass Soft class to load and create.
	 * @param LayerTag Layer to push onto.
	 * @param bSuspendInputUntilComplete Block all input until the widget is on its layer.
	 * @param StateFunc Called as the push moves through EAsyncPushState.
	 * @return Handle of the load. Keep it to cancel the push.
	 */
	template <typename T>
	TSharedPtr<FStreamableHandle> AddWidgetToStackAsync(TSoftClassPtr<UCommonActivatableWidget> WidgetClass, FGameplayTag LayerTag, bool bSuspendInputUntilComplete, TFunction<void(EAsyncPushState, T*)> StateFunc);

	/**
	 * Deactivate the top widget of a layer. For every layer except the bottom one, focus is handed back to the widget below.
	 *
	 * @param LayerTag Layer to pop from.
	 * @return Whether a widget was deactivated.
	 */
	bool PopStack(FGameplayTag LayerTag) const;

	/**
	 * Register a stack as a layer. Call from the widget blueprint for layers beyond the three defaults, bottom to top.
	 *
	 * @param LayerTag Tag the layer is looked up by.
	 * @param Stack Stack widget that holds the layer.
	 */
	UFUNCTION(BlueprintCallable, Category = "Layer")
	void RegisterLayer(UPARAM(meta = (Categories = "Trombone.UI.Layer")) FGameplayTag LayerTag, UCommonActivatableWidgetStack* Stack);

	/** @return The stack registered under the tag, or null. */
	UCommonActivatableWidgetStack* GetLayer(FGameplayTag LayerTag) const;

	/** @return The widget that currently owns UI focus priority. The top most layer with an active widget wins. */
	UCommonActivatableWidget* GetTopActiveWidget() const;

	/** Focus the desired focus target of the top active widget. Used to seed gamepad focus. */
	void FocusActiveWidgetDesiredTarget() const;

	/** Show or hide the FPS and ping overlay. Collapsed widgets do not tick, so hiding also stops the cost. */
	void SetPerformanceWidgetVisible(bool bVisible) const;

	/**
	 * Block every input type for this player until ResumeInput is called with the returned token.
	 *
	 * @param Reason Name that shows up in the input filter. The returned token is this name plus a number.
	 * @return Token to pass to ResumeInput. NAME_None when there is no input subsystem.
	 */
	FName SuspendInput(FName Reason);

	/** Lift a block set by SuspendInput. Safe to call with NAME_None. */
	void ResumeInput(FName Token);

private:

	/** Register the three bound stacks. Runs on initialize and again for the designer preview. */
	void RegisterDefaultLayers();

	/** Handles mouse and gamepad switching. Seeds focus and toggles the cursor for gamepad play. */
	void HandleInputMethodChanged(ECommonInputType NewInputType);

	/** Any layer changed its displayed widget. Refreshes the action bar. */
	void HandleDisplayedWidgetChanged(UCommonActivatableWidget* DisplayedWidget) const;

	/** Shows the action bar for gamepad only, and the accept entry only when something can take focus. */
	void UpdateActionBar() const;

	/** Is the cursor hidden because of gamepad input. Only a cursor we hid gets restored. */
	bool bCursorHiddenForGamepad = false;

	/** Counter that makes every suspend token unique. */
	int32 InputSuspensions = 0;

	/** Layers by tag. */
	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UCommonActivatableWidgetStack>> Layers;

	/** Registration order, bottom first. Focus priority walks it from the back. */
	TArray<FGameplayTag> LayerOrder;

protected:

	/** Bottom layer. Holds the screen of the current level. */
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> BaseStack;

	/** Middle layer. Holds modal popups. */
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> PopupStack;

	/** Top layer. Holds loading and fade overlays. */
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> OverlayStack;

	/** Shown on the Base stack in the widget designer only. Soft so the root does not keep a screen loaded at runtime. */
	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<UCommonActivatableWidget> DefaultWidgetClass;

	/** Placed above the stacks in the root widget. Lives as long as the root, each HUD only toggles it. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPerformanceWidget> PerformanceWidget;

	/** Placed above the stacks in the root widget. Always visible, never hit tested. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProjectVersionWidget> ProjectVersionWidget;

	/** Bottom left action bar. Holds the accept entry and the bound action bar. Shown for gamepad only. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> ActionBar;

	/** The accept entry inside the action bar. Hidden when the active screen has nothing to focus. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> AcceptAction;

	/** Glyph of the accept entry. Set once to the default click action so it follows the input method. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonActionWidget> AcceptActionWidget;

protected:

	//~ Begin UCommonUserWidget Interface
	virtual void NativeOnInitialized() override;
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	//~ End UCommonUserWidget Interface
};

template <typename T>
T* URootUI::AddWidgetToStack(const TSubclassOf<UCommonActivatableWidget> WidgetClass, const FGameplayTag LayerTag, TFunctionRef<void(T&)> InstanceInitFunc) const
{
	if (!WidgetClass) return nullptr;

	UCommonActivatableWidgetStack* TargetStack = GetLayer(LayerTag);
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

template <typename T>
TSharedPtr<FStreamableHandle> URootUI::AddWidgetToStackAsync(const TSoftClassPtr<UCommonActivatableWidget> WidgetClass, const FGameplayTag LayerTag, const bool bSuspendInputUntilComplete, TFunction<void(EAsyncPushState, T*)> StateFunc)
{
	static_assert(std::is_base_of_v<UCommonActivatableWidget, T>, "T must be a child of UCommonActivatableWidget");

	if (WidgetClass.IsNull())
	{
		StateFunc(EAsyncPushState::Canceled, nullptr);
		return nullptr;
	}

	const FName SuspendToken = bSuspendInputUntilComplete ? SuspendInput(TEXT("PushingWidgetToLayer")) : NAME_None;

	FStreamableManager& Streamable = UAssetManager::Get().GetStreamableManager();
	TSharedPtr<FStreamableHandle> Handle = Streamable.RequestAsyncLoad(WidgetClass.ToSoftObjectPath(), FStreamableDelegate::CreateWeakLambda(this,
		[this, WidgetClass, LayerTag, StateFunc, SuspendToken]()
		{
			ResumeInput(SuspendToken);

			T* Widget = AddWidgetToStack<T>(WidgetClass.Get(), LayerTag, [&StateFunc](T& NewWidget)
			{
				StateFunc(EAsyncPushState::Initialize, &NewWidget);
			});
			StateFunc(EAsyncPushState::AfterPush, Widget);
		}));

	// A canceled load must still give the input back
	if (Handle.IsValid())
	{
		Handle->BindCancelDelegate(FStreamableDelegate::CreateWeakLambda(this, [this, StateFunc, SuspendToken]()
		{
			ResumeInput(SuspendToken);
			StateFunc(EAsyncPushState::Canceled, nullptr);
		}));
	}

	return Handle;
}
