#include "UI/UserWidgets/Common/CommonRotatorWidgetBase.h"

void UCommonRotatorWidgetBase::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	RefreshRotator();
}

void UCommonRotatorWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
	
	InitButtons();
}

void UCommonRotatorWidgetBase::SetIsEnabled(const bool bInIsEnabled)
{
	if (CB_Prev)
	{
		CB_Prev->SetIsEnabled(bInIsEnabled);
		CB_Prev->SetVisibility(bInIsEnabled ? ESlateVisibility::Visible : ESlateVisibility::Hidden);		
	}
	if (CB_Next)
	{
		CB_Next->SetIsEnabled(bInIsEnabled);
		CB_Next->SetVisibility(bInIsEnabled ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

#if WITH_EDITOR
void UCommonRotatorWidgetBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
    
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
    
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCommonRotatorWidgetBase, TextOptions))
	{
		RefreshRotator();
	}
}
#endif

void UCommonRotatorWidgetBase::RefreshRotator()
{
	if (CR_Rotator)
	{
		CR_Rotator->PopulateTextLabels(TextOptions);
		CR_Rotator->SetSelectedItem(DefaultSelectedIndex);
	}
}

void UCommonRotatorWidgetBase::SetSelectedIndex(int32 NewIndex)
{
	if (!TextOptions.IsValidIndex(NewIndex)) 
	{
		NewIndex = (DefaultSelectedIndex != -1) ? DefaultSelectedIndex : 0;
	}

	if (CR_Rotator)
	{
		CR_Rotator->SetSelectedItem(NewIndex);
	}
}

void UCommonRotatorWidgetBase::InitButtons()
{
	if (CR_Rotator)
	{
		if (CB_Prev)
		{
			CB_Prev->OnClicked().AddLambda([this]
			{
				CR_Rotator->ShiftTextLeft();
			});
		}
		if (CB_Next)
		{
			CB_Next->OnClicked().AddLambda([this]
			{
				CR_Rotator->ShiftTextRight();
			});
		}
	}
}