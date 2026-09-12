// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/OnScreenIndicator/OSI_InstrumentWidget.h"

#include "Items/ItemBase.h"

void UOSI_InstrumentWidget::SetTrackedItem(AItemBase* InItem)
{
	UnbindTrackedItem();
	TrackedItem = InItem;
	BindTrackedItem();
}

void UOSI_InstrumentWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (IsDesignTime()) return;

	//화면에 붙을 때마다 Construct가 돌므로 여기서 다시 바인딩한다
	BindTrackedItem();
}

void UOSI_InstrumentWidget::NativeDestruct()
{
	//TrackedItem은 비우지 않는다. 다시 붙을 때 재바인딩에 쓴다.
	UnbindTrackedItem();
	Super::NativeDestruct();
}

void UOSI_InstrumentWidget::BindTrackedItem()
{
	AItemBase* Item = TrackedItem.Get();
	if (!Item) return;

	Item->OnOwnerChanged.RemoveDynamic(this, &ThisClass::HandleOwnerChanged);
	Item->OnOwnerChanged.AddDynamic(this, &ThisClass::HandleOwnerChanged);

	//초기 복제가 위젯 생성보다 먼저 올 수 있다. 붙는 순간 현재 값을 직접 읽는다.
	SetIndicatorActive(!Item->IsOwned());
}

void UOSI_InstrumentWidget::UnbindTrackedItem()
{
	if (AItemBase* Item = TrackedItem.Get())
	{
		Item->OnOwnerChanged.RemoveDynamic(this, &ThisClass::HandleOwnerChanged);
	}
}

void UOSI_InstrumentWidget::HandleOwnerChanged(AActor* NewOwner, AActor* OldOwner)
{
	SetIndicatorActive(NewOwner == nullptr);
}
