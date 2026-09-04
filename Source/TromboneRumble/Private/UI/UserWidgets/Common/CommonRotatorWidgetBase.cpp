// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/Common/CommonRotatorWidgetBase.h"
#include "Components/Image.h"

void UCommonRotatorWidgetBase::SetInteractionEnabled(const bool bInEnabled)
{
	if (CB_Prev)
	{
		CB_Prev->SetIsEnabled(bInEnabled);
		CB_Prev->SetVisibility(bInEnabled ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	if (CB_Next)
	{
		CB_Next->SetIsEnabled(bInEnabled);
		CB_Next->SetVisibility(bInEnabled ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void UCommonRotatorWidgetBase::SetSelectedIndex(int32 NewIndex)
{
	if (!TextOptions.IsValidIndex(NewIndex))
	{
		// The default itself can be out of range, so it gets the same check
		NewIndex = TextOptions.IsValidIndex(DefaultSelectedIndex) ? DefaultSelectedIndex : 0;
	}

	if (CR_Rotator)
	{
		CR_Rotator->SetSelectedItem(NewIndex);
	}
	
	ApplyBackground();
}

void UCommonRotatorWidgetBase::SetOptions(const TArray<FText>& InOptions)
{
	TextOptions = InOptions;
	RefreshRotator();
}

void UCommonRotatorWidgetBase::SetOptions(const TArray<FText>& InOptions, const TArray<FSlateBrush>& InBackgrounds)
{
	// Brushes go in first so the refresh inside picks them up right away
	BackgroundBrushes = InBackgrounds;
	SetOptions(InOptions);
}

void UCommonRotatorWidgetBase::BindButtonEvents()
{
	if (CR_Rotator)
	{
		if (CB_Prev)
		{
			CB_Prev->OnClicked().RemoveAll(this);
			CB_Prev->OnClicked().AddWeakLambda(this, [this]
			{
				CR_Rotator->ShiftTextLeft();
			});
		}
		if (CB_Next)
		{
			CB_Next->OnClicked().RemoveAll(this);
			CB_Next->OnClicked().AddWeakLambda(this, [this]
			{
				CR_Rotator->ShiftTextRight();
			});
		}
	}
}

void UCommonRotatorWidgetBase::RefreshRotator()
{
	if (CR_Rotator)
	{
		CR_Rotator->PopulateTextLabels(TextOptions);
		CR_Rotator->SetSelectedItem(DefaultSelectedIndex);
	}
	
	ApplyBackground();
}

void UCommonRotatorWidgetBase::ApplyBackground()
{
	if (!Image_Background)
	{
		return;
	}
	
	const int32 Index = GetCurrentIndex();
	if (BackgroundBrushes.IsValidIndex(Index) && BackgroundBrushes[Index].GetResourceObject())
	{
		Image_Background->SetBrush(BackgroundBrushes[Index]);
		Image_Background->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		Image_Background->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UCommonRotatorWidgetBase::HandleRotatedForBackground(int32 NewIndex, ERotatorDirection Direction)
{
	ApplyBackground();
}

bool UCommonRotatorWidgetBase::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}
	
	BindButtonEvents();
	
	if (CR_Rotator)
	{
		CR_Rotator->OnRotatedWithDirection.AddUniqueDynamic(this, &ThisClass::HandleRotatedForBackground);
	}
	
	return true;
}

#if WITH_EDITOR
void UCommonRotatorWidgetBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCommonRotatorWidgetBase, TextOptions) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(UCommonRotatorWidgetBase, DefaultSelectedIndex))
	{
		RefreshRotator();
	}
	
	// 브러시 내부 필드(틴트, 리소스 등)를 고치면 Property에는 그 필드 이름이 온다. 그래서 소유 멤버 이름으로 비교
	const FName MemberName = (PropertyChangedEvent.MemberProperty != nullptr) ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;
	if (MemberName == GET_MEMBER_NAME_CHECKED(UCommonRotatorWidgetBase, BackgroundBrushes))
	{
		ApplyBackground();
	}
}
#endif

void UCommonRotatorWidgetBase::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	RefreshRotator();
}
