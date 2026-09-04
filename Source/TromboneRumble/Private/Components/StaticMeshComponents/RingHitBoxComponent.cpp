// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/StaticMeshComponents/RingHitBoxComponent.h"
#include "Actors/Rhythm/RhythmActor.h"
#include "Characters/DefaultTromboneCharacter.h"
#include "Subsystems/RhythmSubsystem.h"
#include "GameFramework/Pawn.h"


URingHitBoxComponent::URingHitBoxComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	CanCharacterStepUpOn = ECB_No;
	bReceivesDecals = false;
	SetCastShadow(false);

	// 피아노/계단 등에 가려져도 링이 항상 바닥 위에 그려지도록 translucent 정렬 우선순위 강제
	// (머티리얼 M_NoteHitBox의 Disable Depth Test = true 와 함께 작동)
	SetTranslucentSortPriority(100);

	SetVisibility(false, false);
}

void URingHitBoxComponent::BeginPlay()
{
	Super::BeginPlay();

	EnsureMID();
	ApplyMaterialParams();

	if (RingMID)
	{
		FLinearColor CurrentColor;
		if (RingMID->GetVectorParameterValue(ColorParamName, CurrentColor))
		{
			CachedBaseColor = CurrentColor;
			bHasBaseColor = true;
		}
	}

	//멀티플레이 환경에서 자신만 볼수있게
	const APawn* PawnOwner = Cast<APawn>(GetOwner());
	const bool bShow = PawnOwner && PawnOwner->IsLocallyControlled();

	SetVisibility(bShow, true);
	if (bShow)
	{
		if (URhythmSubsystem* RhythmSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<URhythmSubsystem>())
		{
			RhythmSubsystem->OnNoteDetected.AddDynamic(this, &ThisClass::OnNoteDetectedHandler);
			RhythmSubsystem->OnInstrumentPicked.AddDynamic(this, &ThisClass::OnInstrumentPickedHandler);
		}
		if (ADefaultTromboneCharacter* DefaultTromboneCharacter = Cast<ADefaultTromboneCharacter>(GetOwner()))
		{
			DefaultTromboneCharacter->OnStunStateChanged.AddDynamic(this, &ThisClass::OnOwnerStunnedHandler);
		}
	}
}

void URingHitBoxComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FlashTimerHandle);
	}
	
	Super::EndPlay(EndPlayReason);
}

#if WITH_EDITOR
void URingHitBoxComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	EnsureMID();
	ApplyMaterialParams();

	bHasBaseColor = false;
	CacheBaseColorIfNeeded();

	MarkRenderStateDirty();
}
#endif

void URingHitBoxComponent::OnNoteDetectedHandler(ENoteResult InNoteResult)
{
	switch (InNoteResult)
	{
	case ENoteResult::Bad:
		FlashToColor(BadFlashColor);
		break;
	case ENoteResult::Good:
		FlashToColor(GoodFlashColor);
		break;
	case ENoteResult::Excellent:
		FlashToColor(ExcellentFlashColor);
		break;
	default:
		break;
	}
	
	
}

void URingHitBoxComponent::OnInstrumentPickedHandler(EInstrumentType PrevType, EInstrumentType NewType)
{
	RefreshMaterialFromRhythmActor();

	const APawn* PawnOwner = Cast<APawn>(GetOwner());
	const bool bShow = PawnOwner && PawnOwner->IsLocallyControlled();
	if (PawnOwner && bShow)
	{
		if (NewType == EInstrumentType::Background || EInstrumentType::None<=NewType)
		{
			SetVisibility(false, true);
		}
		else
		{
			SetVisibility(true, true);
		}
	}
}

void URingHitBoxComponent::OnOwnerStunnedHandler(bool bIsStun)
{
	EnsureMID();
	if (!RingMID)
	{
		return;
	}
	CacheBaseColorIfNeeded();
	bIsOwnerStunned = bIsStun;
	if (bIsStun)
	{
		RingMID->SetVectorParameterValue(ColorParamName, StunColor);
	}
	else
	{
		RingMID->SetVectorParameterValue(ColorParamName, CachedBaseColor);
	}
}

void URingHitBoxComponent::EnsureMID()
{
	// 이미 만들어져 있으면 재사용
	if (RingMID || !GetWorld()) return;

	// 맵별 머티리얼 > RingMatOrigin > 슬롯 0 순으로 사용
	UMaterialInterface* BaseMat = ResolvePerMapOverrideMaterial();
	if (!BaseMat)
	{
		BaseMat = RingMatOrigin ? RingMatOrigin.Get() : GetMaterial(0);
	}
	if (!BaseMat) return;

	RingMID = CreateDynamicMaterialInstance(0, BaseMat);

	if (RingMID)
	{
		CachedParentMat = BaseMat;
		ApplyMaterialParams();
	}
}

UMaterialInterface* URingHitBoxComponent::ResolvePerMapOverrideMaterial() const
{
	const UWorld* World = GetWorld();
	if (!World || !World->GetGameInstance()) return nullptr;

	const URhythmSubsystem* RhythmSubsystem = World->GetGameInstance()->GetSubsystem<URhythmSubsystem>();
	if (!RhythmSubsystem) return nullptr;

	const ARhythmActor* CurrentRhythmActor = RhythmSubsystem->GetRegisteredRhythmActor(World);
	return CurrentRhythmActor ? CurrentRhythmActor->GetHitBoxRingMaterial() : nullptr;
}

void URingHitBoxComponent::RefreshMaterialFromRhythmActor()
{
	UMaterialInterface* OverrideMat = ResolvePerMapOverrideMaterial();
	// 이미 해당 머티리얼로 MID를 만들었거나, 맵에 지정된 머티리얼이 없으면 그대로 둔다
	if (!OverrideMat || OverrideMat == CachedParentMat) return;

	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FlashTimerHandle);
	}

	RingMID = nullptr;
	bHasBaseColor = false;

	EnsureMID();
	CacheBaseColorIfNeeded();

	if (bIsOwnerStunned && RingMID)
	{
		RingMID->SetVectorParameterValue(ColorParamName, StunColor);
	}
}

void URingHitBoxComponent::ApplyMaterialParams()
{
	if (!RingMID)
	{
		UE_LOG(LogTemp, Warning, TEXT("No RingMID"));
		return;
	}
	RingMID->SetScalarParameterValue(TEXT("StartOuterRadius"), StartOuterRadius);
	RingMID->SetScalarParameterValue(TEXT("StartInnerRadius"), StartInnerRadius);
	RingMID->SetScalarParameterValue(TEXT("EndOuterRadius"), EndOuterRadius);
	RingMID->SetScalarParameterValue(TEXT("EndInnerRadius"), EndInnerRadius);
	RingMID->SetScalarParameterValue(TEXT("SizeAlpha"), SizeAlpha);
}

void URingHitBoxComponent::CacheBaseColorIfNeeded()
{
	if (bHasBaseColor)
	{
		return;
	}

	EnsureMID();
	if (!RingMID)
	{
		return;
	}

	// 머티리얼에서 현재 색을 읽어 원본색으로 사용
	FLinearColor Current;
	if (RingMID->GetVectorParameterValue(ColorParamName, Current))
	{
		CachedBaseColor = Current;
	}
	else
	{
		CachedBaseColor = FLinearColor::White;
	}

	bHasBaseColor = true;
}

void URingHitBoxComponent::FlashToColor(const FLinearColor& InColor)
{
	EnsureMID();
	if (!RingMID)
	{
		return;
	}

	CacheBaseColorIfNeeded();

	// 기존 복귀 타이머 제거(연타 대응)
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FlashTimerHandle);
	}

	RingMID->SetVectorParameterValue(ColorParamName, InColor);
	
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			FlashTimerHandle,
			this,
			&URingHitBoxComponent::RestoreBaseColor,
			FlashDuration,
			false
		);
	}
}

void URingHitBoxComponent::RestoreBaseColor()
{
	if (!RingMID)
	{
		return;
	}

	if (bIsOwnerStunned)
	{
		RingMID->SetVectorParameterValue(ColorParamName, StunColor);
	}
	else
	{
		RingMID->SetVectorParameterValue(ColorParamName, CachedBaseColor);
	}
	
}
