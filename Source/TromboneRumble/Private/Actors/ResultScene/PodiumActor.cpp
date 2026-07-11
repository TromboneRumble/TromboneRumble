// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/ResultScene/PodiumActor.h"
#include "Utilities/Defines.h"
#include "Components/WidgetComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/ActorComponents/CustomizationComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UI/UserWidgets/InGame/PodiumNameWidget.h"

APodiumActor::APodiumActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	NameWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("NameWidgetComponent"));
	if (NameWidgetComponent)
	{
		NameWidgetComponent->SetupAttachment(RootComponent);
		NameWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
		NameWidgetComponent->SetDrawAtDesiredSize(true);
	}

	// 인게임 캐릭터와 동일한 모듈식 외형(Antenna/Costume follower + Face 교체)을 재사용
	CustomizationComp = CreateDefaultSubobject<UCustomizationComponent>(TEXT("CustomizationComponent"));
}

void APodiumActor::EnsureHeadMaterialInstances()
{
	if (!MeshComponent) return;

	if (!SkinMID)
	{
		// 슬롯 이름으로 조회 → MID 없으면 생성 (TromboneCharacterBase::BeginPlay와 동일 패턴)
		SkinMID = UCustomizationComponent::EnsureSlotMID(MeshComponent, TromboneMaterial::SkinSlotName);
	}

	if (!FaceMID)
	{
		const int32 FaceIndex = MeshComponent->GetMaterialIndex(TromboneMaterial::FaceSlotName);
		if (FaceIndex != INDEX_NONE)
		{
			// MID 생성 전 원본 face 머티리얼을 최초 1회만 캐싱 (커마 Face가 nullptr일 때 복원용)
			if (!OriginalFaceMaterial.IsValid())
			{
				OriginalFaceMaterial = MeshComponent->GetMaterial(FaceIndex);
			}
			FaceMID = UCustomizationComponent::EnsureSlotMID(MeshComponent, TromboneMaterial::FaceSlotName);
		}
	}
}

void APodiumActor::ApplySkinColor(const FLinearColor& InSkinColor)
{
	// follower 파츠/Face 틴트가 참조하므로 먼저 저장
	CachedSkinColor = InSkinColor;
	// 디렉터가 포디움 BeginPlay보다 먼저 호출해도 머리에 색이 입혀지도록 MID를 확보
	EnsureHeadMaterialInstances();
	if (SkinMID) SkinMID->SetVectorParameterValue(TromboneMaterial::BaseColorParam, InSkinColor);
	if (FaceMID) FaceMID->SetVectorParameterValue(TromboneMaterial::BaseColorParam, InSkinColor);
}

void APodiumActor::ApplyCustomization(const FCustomizationSaveData& Data)
{
	// 피부색은 컴포넌트가 GetOwnerSkinColor()→GetSkinColor()로 읽어 follower에 자동 틴트하므로
	// ApplySkinColor가 먼저 호출되어 CachedSkinColor가 채워진 상태여야 한다.
	if (CustomizationComp)
	{
		CustomizationComp->LoadFromSaveData(Data);
	}
}

void APodiumActor::ApplyFaceMaterial(UMaterialInterface* Material)
{
	// nullptr 전달 시 캐싱된 원본 머티리얼로 복원 (ATromboneCharacterBase::ApplyFaceMaterial 패턴)
	UMaterialInterface* Target = Material ? Material : OriginalFaceMaterial.Get();
	if (!Target || !MeshComponent) return;

	const int32 FaceIndex = MeshComponent->GetMaterialIndex(TromboneMaterial::FaceSlotName);
	if (FaceIndex == INDEX_NONE) return;

	MeshComponent->SetMaterial(FaceIndex, Target);
	FaceMID = MeshComponent->CreateAndSetMaterialInstanceDynamic(FaceIndex);
	if (FaceMID)
	{
		// 스킨 틴트는 항상 재적용
		FaceMID->SetVectorParameterValue(TromboneMaterial::BaseColorParam, CachedSkinColor);
		// 표정은 우는 포디움(4등)만 재적용 → 1~3등은 커마 face 기본 표정 그대로 유지
		if (bIsCrying)
		{
			FaceMID->SetScalarParameterValue(FaceExpressionParameterName, static_cast<float>(CurrentFaceType));
		}
	}
}

void APodiumActor::SetPlayerName(const FString& InName)
{
	if (NameWidgetComponent)
	{
		if (UPodiumNameWidget* NameWidget = Cast<UPodiumNameWidget>(NameWidgetComponent->GetUserWidgetObject()))
		{
			NameWidget->SetPlayerName(InName);
		}
	}
}

void APodiumActor::SetNameWidgetVisibility(bool bVisible)
{
	if (NameWidgetComponent)
	{
		NameWidgetComponent->SetVisibility(bVisible);
	}
}


void APodiumActor::BeginPlay()
{
	Super::BeginPlay();

	// 디렉터가 ApplySkinColor/ApplyCustomization을 먼저 호출했다면 이미 확보돼 있고,
	// 아니라면 여기서 생성된다 (멱등). 호출 순서와 무관하게 머리 색이 보장됨.
	EnsureHeadMaterialInstances();

	// 1~3등은 표정 구동 없이 커마 face 기본 표정 유지, 4등(bIsCrying)만 Cry 재생
	if (bIsCrying)
	{
		PlayFaceSequence(ECharacterFaceState::Cry);
	}

	if (NameWidgetComponent)
	{
		NameWidgetComponent->SetVisibility(false);
	}
}

void APodiumActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorldTimerManager().ClearTimer(FaceSequenceTimerHandle);
		FaceSequenceTimerHandle.Invalidate();
	}
	Super::EndPlay(EndPlayReason);
}

void APodiumActor::PlayFaceSequence(ECharacterFaceState TargetState)
{
	if (!CharacterData) return;

	if (const FCharacterFaceAnimationSequence* FaceAnimData = CharacterData->FaceSequences.Find(TargetState))
	{
		InternalPlayFaceSequence(FaceAnimData);
	}
}

void APodiumActor::InternalPlayFaceSequence(const FCharacterFaceAnimationSequence* InSequence)
{
	GetWorld()->GetTimerManager().ClearTimer(FaceSequenceTimerHandle);
	CurrentActiveSequence = *InSequence;
	CurrentSequenceStep = 0;
	ExecuteFaceStep();
}

void APodiumActor::ExecuteFaceStep()
{
	if (CurrentActiveSequence.Sequence.Num() == 0) return;

	UpdateFaceExpression(CurrentActiveSequence.Sequence[CurrentSequenceStep]);
	CurrentSequenceStep++;

	if (CurrentSequenceStep < CurrentActiveSequence.Sequence.Num())
	{
		GetWorld()->GetTimerManager().SetTimer(FaceSequenceTimerHandle, this, &ThisClass::ExecuteFaceStep, CurrentActiveSequence.Interval, false);
	}
	else if (CurrentActiveSequence.bLoop)
	{
		CurrentSequenceStep = 0;
		float NextDelay = FMath::FRandRange(CurrentActiveSequence.MinLoopDelay, CurrentActiveSequence.MaxLoopDelay);
		if (NextDelay <= 0.0f) NextDelay = CurrentActiveSequence.Interval;

		GetWorld()->GetTimerManager().SetTimer(FaceSequenceTimerHandle, this, &ThisClass::ExecuteFaceStep, NextDelay, false);
	}
}

void APodiumActor::UpdateFaceExpression(ECharacterFaceType NewType)
{
	// 커마 Face 교체로 FaceMID가 재생성되어도 표정을 복원할 수 있게 현재 값 기록
	CurrentFaceType = NewType;
	if (FaceMID)
	{
		// Material의 ExpressionIndex 파라미터 업데이트
		FaceMID->SetScalarParameterValue(FaceExpressionParameterName, static_cast<float>(NewType));
	}
}


