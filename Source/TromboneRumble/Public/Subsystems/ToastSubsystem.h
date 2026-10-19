// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/ToastData.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ToastSubsystem.generated.h"

class UToastContainerWidget;
class UToastItemWidget;

/** Delegate triggered after a travel, right before the toasts waiting for the new map are shown. */
DECLARE_MULTICAST_DELEGATE(FOnCollectToastsAfterTravel);

UCLASS()
class TROMBONERUMBLE_API UToastSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	//~ Begin USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End USubsystem interface

	/** Get toast subsystem */
	static UToastSubsystem* Get(const UObject* WorldContextObject);

	/** Shows the toast of a notice. Its text is the entry of the notice string table in Toast Settings whose key is the notice tag's name. */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Toast", meta = (WorldContext = "WorldContextObject"))
	static void ShowToast(const UObject* WorldContextObject, UPARAM(meta = (Categories = "Trombone.UI.Notice")) FGameplayTag Notice);

	/** Shows the toast of a notice once the next map finished loading, for a travel that starts now. */
	static void ShowToastAfterTravel(const UObject* WorldContextObject, const FGameplayTag& Notice);

	/** Return a toast widget to the pool */
	void ReturnToPool(UToastItemWidget* ToastWidget);

	/**
	 * Triggered when a map other than the transition map finished loading, right before the waiting toasts are shown.
	 * Call ShowToastAfterTravel from a handler to show a toast on the new map.
	 */
	FOnCollectToastsAfterTravel OnCollectToastsAfterTravel;

private:

	/** Handle when changing level */
	void OnPostLoadMap(UWorld* NewWorld);

	/** Puts the toast of a notice in the queue, and shows it now when no other toast is showing. */
	void QueueToast(const FGameplayTag& Notice);

	void ProcessNextToast();

	void ProcessNextToastInternal(const FToastRequest& InRequest);

	/** @return toast widget from the pool or create a new one */
	UToastItemWidget* GetOrCreateToastWidget();

	/** Must clear toast pool on level changed */
	void ClearToastPool();

	UPROPERTY()
	TSubclassOf<UToastContainerWidget> ToastContainerClass;

	UPROPERTY()
	TSubclassOf<UToastItemWidget> SimpleToastItemClass;

	UPROPERTY()
	TObjectPtr<UToastContainerWidget> ToastContainer;

	UPROPERTY()
	TArray<TObjectPtr<UToastItemWidget>> ToastPool;

	TQueue<FToastRequest> ToastQueue;

	/** Notices waiting for the next map. Kept through the transition map of a seamless travel. */
	TArray<FGameplayTag> ToastsAfterTravel;

	/** Queue or override */
	EToastSystemPolicy ToastPolicy = EToastSystemPolicy::Queue;

	bool bIsToastShowing = false;
};
