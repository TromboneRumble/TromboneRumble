// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "UI/UserWidgets/Common/Toast/ToastContainerWidget.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Data/UIData.h"

void UToastContainerWidget::AddToastItem(UUserWidget* InToastItem, const EToastPosition InPosition)
{
    if (!InToastItem || !ToastAnchorOverlay) return;

    ToastAnchorOverlay->ClearChildren();
    
    UPanelSlot* NewSlot = ToastAnchorOverlay->AddChild(InToastItem);
    CurrentActiveToast = InToastItem;
    
    if (UOverlaySlot* OverlaySlot = Cast<UOverlaySlot>(NewSlot))
    {
        switch (InPosition)
        {
            case EToastPosition::TopLeft:
                OverlaySlot->SetHorizontalAlignment(HAlign_Left);
                OverlaySlot->SetVerticalAlignment(VAlign_Top);
                break;
            
            case EToastPosition::TopCenter:
                OverlaySlot->SetHorizontalAlignment(HAlign_Center);
                OverlaySlot->SetVerticalAlignment(VAlign_Top);
                break;
            
            case EToastPosition::TopRight:
                OverlaySlot->SetHorizontalAlignment(HAlign_Right);
                OverlaySlot->SetVerticalAlignment(VAlign_Top);
                break;
            
            case EToastPosition::CenterLeft:
                OverlaySlot->SetHorizontalAlignment(HAlign_Left);
                OverlaySlot->SetVerticalAlignment(VAlign_Center);
                break;
            
            case EToastPosition::Center:
                OverlaySlot->SetHorizontalAlignment(HAlign_Center);
                OverlaySlot->SetVerticalAlignment(VAlign_Center);
                break;
            
            case EToastPosition::CenterRight:
                OverlaySlot->SetHorizontalAlignment(HAlign_Right);
                OverlaySlot->SetVerticalAlignment(VAlign_Center);
                break;
            
            case EToastPosition::BottomLeft:
                OverlaySlot->SetHorizontalAlignment(HAlign_Left);
                OverlaySlot->SetVerticalAlignment(VAlign_Bottom);
                break;
            
            case EToastPosition::BottomCenter:
                OverlaySlot->SetHorizontalAlignment(HAlign_Center);
                OverlaySlot->SetVerticalAlignment(VAlign_Bottom);
                break;
            
            case EToastPosition::BottomRight:
                OverlaySlot->SetHorizontalAlignment(HAlign_Right);
                OverlaySlot->SetVerticalAlignment(VAlign_Bottom);
                break;
            
            case EToastPosition::Random:
                {
                    const int32 MaxIndex = static_cast<int32>(EToastPosition::MAX) - 1;
                    const int32 RandomIndex = FMath::RandRange(0, MaxIndex);
                    const EToastPosition RandomPosition = static_cast<EToastPosition>(RandomIndex);
                    AddToastItem(InToastItem, RandomPosition);
                    return;
                }
            
            default:
                OverlaySlot->SetHorizontalAlignment(HAlign_Center);
                OverlaySlot->SetVerticalAlignment(VAlign_Center);
                break;
        }
    }
}

UUserWidget* UToastContainerWidget::GetCurrentActiveToast()
{
    return CurrentActiveToast.IsValid() ? CurrentActiveToast.Get() : nullptr;
}
