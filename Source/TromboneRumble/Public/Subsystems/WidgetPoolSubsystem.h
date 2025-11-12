// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h" 
#include "Components/SlateWrapperTypes.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "WidgetPoolSubsystem.generated.h"

class UCanvasPanel;
class UPanelWidget;

USTRUCT()
struct FWidgetPoolKey
{
	GENERATED_BODY()

	UPROPERTY() TSubclassOf<UUserWidget> Class;
	UPROPERTY() TWeakObjectPtr<UCanvasPanel> Canvas; // 또는 Parent UserWidget

	bool operator==(const FWidgetPoolKey& Other) const
	{
		return Class.Get() == Other.Class.Get() && Canvas == Other.Canvas;
	}
};

FORCEINLINE uint32 GetTypeHash(const FWidgetPoolKey& K)
{
	return HashCombine(GetTypeHash(K.Class.Get()),
		GetTypeHash(K.Canvas.Get()));
}

USTRUCT()
struct FWidgetPool
{
	GENERATED_BODY()

	UPROPERTY() TSubclassOf<UUserWidget> Class;
	UPROPERTY() TArray<TObjectPtr<UUserWidget>> Inactive;	// 비활성 큐
	UPROPERTY() TSet<TObjectPtr<UUserWidget>> Active;		// 활성 집합
	int32 TotalCreated = 0;
};

/**
 * 
 */
UCLASS()
class TROMBONERUMBLE_API UWidgetPoolSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
public:
	void Prewarm(TSubclassOf<UUserWidget> InClass, int32 InCount, UUserWidget* InParent, UCanvasPanel* InCanvas, ESlateVisibility InVisible = ESlateVisibility::HitTestInvisible);

	UUserWidget* Acquire(TSubclassOf<UUserWidget> InClass, UUserWidget* InParent, UCanvasPanel* InCanvas);

	void Release(UUserWidget* InWidget);

private:
	UPROPERTY() TMap<FWidgetPoolKey, FWidgetPool> Pools;
	UUserWidget* SpawnPooledWidget(TSubclassOf<UUserWidget> InClass, UUserWidget* InParent, UCanvasPanel* InCanvas);
	void Activate(UUserWidget* InWidget);
	void Deactivate(UUserWidget* InWidget);
};
