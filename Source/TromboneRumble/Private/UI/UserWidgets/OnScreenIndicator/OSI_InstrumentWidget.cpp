// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/OnScreenIndicator/OSI_InstrumentWidget.h"

#include "Items/ItemBase.h"
#include "Subsystems/RhythmSubsystem.h"
#include "Utilities/Defines.h"

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

	if (URhythmSubsystem* RhythmSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<URhythmSubsystem>() : nullptr)
	{
		RhythmSubsystem->OnInstrumentPicked.RemoveDynamic(this, &ThisClass::HandleInstrumentPicked);
		RhythmSubsystem->OnInstrumentPicked.AddDynamic(this, &ThisClass::HandleInstrumentPicked);
	}

	//초기 복제가 위젯 생성보다 먼저 올 수 있다. 붙는 순간 현재 값을 직접 읽는다.
	RefreshIndicator();
}

void UOSI_InstrumentWidget::UnbindTrackedItem()
{
	if (AItemBase* Item = TrackedItem.Get())
	{
		Item->OnOwnerChanged.RemoveDynamic(this, &ThisClass::HandleOwnerChanged);
	}

	if (URhythmSubsystem* RhythmSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<URhythmSubsystem>() : nullptr)
	{
		RhythmSubsystem->OnInstrumentPicked.RemoveDynamic(this, &ThisClass::HandleInstrumentPicked);
	}
}

void UOSI_InstrumentWidget::RefreshIndicator()
{
	AItemBase* Item = TrackedItem.Get();
	if (!Item) return;

	//"내가 지금 집을 수 있다"는 표시라 줍기 판정을 그대로 쓴다
	//폰이 아직 없으면 CanInteract가 null을 역참조하므로 소유 여부만 본다
	APawn* LocalPawn = GetOwningPlayerPawn();
	SetIndicatorActive(LocalPawn ? IInteractable::Execute_CanInteract(Item, LocalPawn) : !Item->IsOwned());
}

void UOSI_InstrumentWidget::HandleOwnerChanged(AActor* NewOwner, AActor* OldOwner)
{
	RefreshIndicator();
}

void UOSI_InstrumentWidget::HandleInstrumentPicked(EInstrumentType PrevType, EInstrumentType NewType)
{
	RefreshIndicator();
}
