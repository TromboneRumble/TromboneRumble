// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgets/OnScreenIndicator/OSI_WidgetBase.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Utilities/DebugHelper.h"


void UOSI_WidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            TimerHandle_Update,
            this,
            &UOSI_WidgetBase::OSITimer,
            0.005f,
            true   
        );
    }
}

void UOSI_WidgetBase::NativeDestruct()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(TimerHandle_Update);
    }
	Super::NativeDestruct();
}

void UOSI_WidgetBase::OSITimer()
{
    if (!TargetComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("No Target Component"));
    }
    UpdateViewportSize();
    ObjectLocation = TargetComponent->GetComponentLocation();
    //물체가 화면 내에 있는 경우
    if (IsWorldLocationWithinScreenClamp(ObjectLocation))
    {
        //물체가 화면 좌표계 내에 있을 경우엔 위치만 Update 시킴
        UpdateWidgetLocation(true);
    }
    //물체가 화면 바깥에 있는 경우
    else
    {
        APlayerController* PlayerController = GetOwningPlayer();
        FVector DummyWorldDirection;
        //ScreenSpace에서의 ViewPort 중앙 지점 -> WorldSpace에서의 위치(MiddlePoint)로 변환
        UGameplayStatics::DeprojectScreenToWorld(PlayerController, SavedViewportSize / FVector2D(2.0f, 2.0f), MiddlePoint, DummyWorldDirection);
        //화면 중앙 -> 물체로 향하는 방향벡터
        ObjectDirection = UKismetMathLibrary::GetDirectionUnitVector(MiddlePoint, ObjectLocation);
        //화면 중앙 + 방향벡터 * 2;
        MidPointTowardObject = MiddlePoint + ObjectDirection * Accuracy;

        //Viewport가 매우 작을 경우에 대한 예외 처리
        if (!IsWorldLocationWithinScreenClamp(MidPointTowardObject))
        {
            UE_LOG(LogTemp, Warning, TEXT("Mid Point Toward Object Not On Screen"));
            return;
        }


        //ScreenSpace->UMGScreenSpace의 중앙지점
        ScreenMiddle2D = SavedViewportSize / UWidgetLayoutLibrary::GetViewportScale(this) / FVector2D(2.0, 2.0);
        RunAndRise = ScreenMiddle2D - WidgetScreenLocation;
        LineLength = UKismetMathLibrary::GetMin2D(UKismetMathLibrary::GetAbs2D((ScreenMiddle2D - ClampMin) / RunAndRise));
        WidgetScreenLocation = ScreenMiddle2D - RunAndRise * LineLength;
        UpdateWidgetLocation(false);
    }
}

void UOSI_WidgetBase::UpdateViewportSize()
{
    FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(this);
    if (!SavedViewportSize.Equals(ViewportSize))
    {
        SavedViewportSize = ViewportSize;
        //ViewPort 좌상단에서 Image Size의 절반만큼씩 더함.
        ClampMin = UWidgetLayoutLibrary::SlotAsCanvasSlot(IndicatorIcon)->GetSize() / FVector2D(2.0f, 2.0f);
        //ViewPort 우하단에서 Image의 size만큼 뺀 위치
        ClampMax = SavedViewportSize / UWidgetLayoutLibrary::GetViewportScale(this) - ClampMin;
    }
}

bool UOSI_WidgetBase::IsWorldLocationWithinScreenClamp(const FVector& InWorldPosition)
{
    //InWorldPosition을 PlayerCamera에 투영했을때, 해당 위치를 ScreenSpace로 WidgetScreenLocation으로 저장
    //만약 WidgetScreenLocation이 화면 내부에 있으면 true 반환
    const bool isObjectWithinDeadZone = UGameplayStatics::ProjectWorldToScreen(GetOwningPlayer(), InWorldPosition, WidgetScreenLocation, true);
    //ScreenSpace ->UMGScreenSpace로 변환
    WidgetScreenLocation = WidgetScreenLocation / UWidgetLayoutLibrary::GetViewportScale(this);
    const bool isWidgetScreenLocationInClampValueX = UKismetMathLibrary::InRange_FloatFloat(WidgetScreenLocation.X, ClampMin.X, ClampMax.X);
    const bool isWidgetScreenLocationInClampValueY = UKismetMathLibrary::InRange_FloatFloat(WidgetScreenLocation.Y, ClampMin.Y, ClampMax.Y);

    
    if (isObjectWithinDeadZone && isWidgetScreenLocationInClampValueX && isWidgetScreenLocationInClampValueY) return true;
    return false;
}

void UOSI_WidgetBase::UpdateWidgetLocation(bool IsOnScreen)
{
    //IndicatorIcon 위치 조정
    WidgetScreenLocation.X = UKismetMathLibrary::FClamp(WidgetScreenLocation.X, ClampMin.X, ClampMax.X);
    WidgetScreenLocation.Y = UKismetMathLibrary::FClamp(WidgetScreenLocation.Y, ClampMin.Y, ClampMax.Y);
    UWidgetLayoutLibrary::SlotAsCanvasSlot(IndicatorIcon)->SetPosition(WidgetScreenLocation);
    if (TargetIcon)
    {
        UWidgetLayoutLibrary::SlotAsCanvasSlot(TargetIcon)->SetPosition(WidgetScreenLocation);
    }

    UpdateSpriteAngle(IsOnScreen);
}

void UOSI_WidgetBase::UpdateSpriteAngle(bool IsOnScreen)
{
    if (IsOnScreen)
    {
        IndicatorIcon->SetBrushFromTexture(NonPointingIndicatorTex);

        if (bShowWidgetWhenInScreen)
        {
            IndicatorIcon->SetVisibility(ESlateVisibility::Collapsed);
            if (TargetIcon) TargetIcon->SetVisibility(ESlateVisibility::Collapsed);
        }
        else
        {
            IndicatorIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
            if (TargetIcon) TargetIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
        IndicatorIcon->SetRenderTransformAngle(0.0f);
    }
    else
    {
        IndicatorIcon->SetBrushFromTexture(PointingIndicatorTex);
        IndicatorIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        if (TargetIcon)
        {
            TargetIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        }

        float ScreenMiddle2DMinVal = UKismetMathLibrary::FMin(ScreenMiddle2D.X, ScreenMiddle2D.Y);
        FVector StartVec(ScreenMiddle2DMinVal, ScreenMiddle2DMinVal, 0.f);

        float TargetX = UKismetMathLibrary::NormalizeToRange(WidgetScreenLocation.X, ClampMin.X, ClampMax.X);
        TargetX = UKismetMathLibrary::Lerp(UKismetMathLibrary::FMin(ClampMin.X, ClampMin.Y), UKismetMathLibrary::FMin(ClampMax.X, ClampMax.Y), TargetX);

        float TargetY = UKismetMathLibrary::NormalizeToRange(WidgetScreenLocation.Y, ClampMin.Y, ClampMax.Y);
        TargetY = UKismetMathLibrary::Lerp(UKismetMathLibrary::FMin(ClampMin.X, ClampMin.Y), UKismetMathLibrary::FMin(ClampMax.X, ClampMax.Y), TargetY);


        FRotator ScreenMiddleToTargetRot = UKismetMathLibrary::FindLookAtRotation(FVector(ScreenMiddle2DMinVal, ScreenMiddle2DMinVal, 0.f), FVector(TargetX, TargetY, 0.f));

        IndicatorIcon->SetRenderTransformAngle(ScreenMiddleToTargetRot.Yaw + 90.0f);
    }
}
