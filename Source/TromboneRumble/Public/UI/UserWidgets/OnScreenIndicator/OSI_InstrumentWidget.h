// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OSI_WidgetBase.h"
#include "OSI_InstrumentWidget.generated.h"

class AItemBase;

/**
 * 바닥에 떨어진 아이템을 가리키는 인디케이터.
 * 누구든 아이템을 집으면 스스로 숨고, 놓으면 다시 나타난다.
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

private:
	void BindTrackedItem();
	void UnbindTrackedItem();

	UPROPERTY(Transient)
	TWeakObjectPtr<AItemBase> TrackedItem = nullptr;
};
