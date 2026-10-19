// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/UIData.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ToastSubsystem.generated.h"

enum class EToastSystemPolicy : uint8;
class UToastContainerWidget;
class UToastItemWidget;

UCLASS()
class TROMBONERUMBLE_API UToastSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	
	/** Default constructor */
	UToastSubsystem();
	
	/** Get toast subsystem */
	UFUNCTION(BlueprintPure, Category = "Trombone|Subsystem", DisplayName = "Get Toast Subsystem", meta = (WorldContext = "WorldContextObject"))
	static UToastSubsystem* Get(const UObject* WorldContextObject);
	
	/** Show a toast message with the given request data. */
	void ShowToast(const FToastRequest& InRequest);

	/** Show the toast of a notice now. Its text is the ST_Common entry whose key is the notice tag's name. */
	void ShowNotice(const FGameplayTag& Notice);

	/** Show the toast of a notice once the next map finished loading, for a travel that starts now. */
	void ShowNoticeAfterTravel(const FGameplayTag& Notice);
	
	/** Return a toast widget to the pool */
	void ReturnToPool(UToastItemWidget* ToastWidget);

private:
	
	/** @return toast widget from the pool or create a new one */
	UToastItemWidget* GetOrCreateToastWidget();
	
	void ProcessNextToastInternal(const FToastRequest& InRequest);
	
	void ProcessNextToast();
	
	/** Handle when changing level */
	void OnPostLoadMap(UWorld* NewWorld);
	
	/** Must clear toast pool on level changed */
	void ClearToastPool();

private:
	
	UPROPERTY()
	TSubclassOf<UToastItemWidget> SimpleToastItemClass;
	
	UPROPERTY()
	TSubclassOf<UToastContainerWidget> ToastContainerClass;
	
private:
	
	UPROPERTY()
	TObjectPtr<UToastContainerWidget> ToastContainer;
	
	UPROPERTY()
	TArray<TObjectPtr<UToastItemWidget>> ToastPool;
	
	TQueue<FToastRequest> ToastQueue;

	/** Notices waiting for the next map. Kept through the transition map of a seamless travel. */
	TArray<FGameplayTag> NoticesAfterTravel;
	
	/** Queue or override */
	EToastSystemPolicy ToastPolicy;
	
	bool bIsToastShowing;
	
public:
	
	// ~ Begin USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// ~ End USubsystem interface
	
};
