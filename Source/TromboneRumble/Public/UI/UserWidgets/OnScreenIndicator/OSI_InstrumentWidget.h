// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OSI_WidgetBase.h"
#include "OSI_InstrumentWidget.generated.h"

class AItemBase;
enum class EInstrumentType : uint8;

/**
 * 바닥에 떨어진 아이템을 가리키는 인디케이터.
 * 로컬 플레이어가 지금 집을 수 있을 때만 보인다.
 * 누가 들고 있거나, 내가 이미 악기를 들고 있으면 숨는다.
 */
UCLASS(Abstract, meta = (DisableNativeTick))
class TROMBONERUMBLE_API UOSI_InstrumentWidget : public UOSI_WidgetBase
{
	GENERATED_BODY()

public:
	// 추적할 아이템을 물린다. 바인딩과 현재 소유 상태 반영을 같이 한다.
	void SetTrackedItem(AItemBase* InItem);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void HandleOwnerChanged(AActor* NewOwner, AActor* OldOwner);

	// 로컬 플레이어의 악기가 바뀌면 줍기 가능 여부도 바뀐다
	UFUNCTION()
	void HandleInstrumentPicked(EInstrumentType PrevType, EInstrumentType NewType);

private:
	void BindTrackedItem();
	void UnbindTrackedItem();
	void RefreshIndicator();

	UPROPERTY(Transient)
	TWeakObjectPtr<AItemBase> TrackedItem = nullptr;
};
