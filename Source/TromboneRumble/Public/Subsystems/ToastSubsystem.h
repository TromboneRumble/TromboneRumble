// Copyright (C) 2026 biksari studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ToastSubsystem.generated.h"

enum class EToastSystemPolicy : uint8;
class UToastContainerWidget;
class UToastItemWidget;
struct FToastRequest;

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
	
	/** Queue or override */
	EToastSystemPolicy ToastPolicy;
	
	bool bIsToastShowing;
	
public:
	
	// ~ Begin USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// ~ End USubsystem interface
	
};
