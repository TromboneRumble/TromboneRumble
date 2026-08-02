// Copyright (C) 2026 biksari studio. All Rights Reserved.

#include "Components/ActorComponents/XRayComponentBase.h"
#include "Camera/CameraComponent.h"
#include "Characters/TromboneCharacterBase.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UXRayComponentBase::UXRayComponentBase()
{
	PrimaryComponentTick.bCanEverTick = true;
	// 원격 캐릭터에서는 아무것도 하지 않는다. TryStartTracking만 틱을 켠다
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UXRayComponentBase::BeginPlay()
{
	Super::BeginPlay();

	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		// 클라이언트는 BeginPlay 시점에 아직 조종권이 없을 수 있다
		OwnerPawn->ReceiveControllerChangedDelegate.AddDynamic(this, &UXRayComponentBase::HandleControllerChanged);
	}

	if (ATromboneCharacterBase* OwnerCharacter = Cast<ATromboneCharacterBase>(GetOwner()))
	{
		OwnerCharacter->OnSkinColorChanged.AddDynamic(this, &UXRayComponentBase::HandleSkinColorChanged);
		// 피부색은 PlayerState 복제 경로로도 오므로 순서를 가정하지 않고 현재 값을 한 번 당겨온다
		OnSkinColorChanged(OwnerCharacter->GetSkinColor());
	}

	TryStartTracking();
}

void UXRayComponentBase::TryStartTracking()
{
	if (bEffectActive || bTrackingPaused)
	{
		return;
	}

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled())
	{
		return;
	}

	StartOcclusionTracking(GetOwner()->FindComponentByClass<UCameraComponent>());
}

void UXRayComponentBase::StartOcclusionTracking(UCameraComponent* InCamera)
{
	if (!InCamera || bEffectActive)
	{
		return;
	}

	Camera = InCamera;

	if (!InitializeEffect())
	{
		// 초기화 실패(머티리얼 미지정 등)면 잠든 채로 둔다. 다른 X-Ray 컴포넌트를 방해하지 않기 위함
		return;
	}

	bEffectActive = true;
	TimeSinceTrace = TraceInterval;	// 첫 틱에 즉시 판정
	SetComponentTickEnabled(true);
}

void UXRayComponentBase::HandleControllerChanged(APawn* InPawn, AController* OldController, AController* NewController)
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	const bool bLocallyControlled = OwnerPawn && OwnerPawn->IsLocallyControlled();

	if (bLocallyControlled)
	{
		TryStartTracking();
		return;
	}

	// 조종권을 잃었는데 효과가 켜져 있으면 화면에 흔적이 남는다
	if (bEffectActive)
	{
		SetComponentTickEnabled(false);
		TeardownEffect();
		bEffectActive = false;
		bAnyOccluding = false;
		Camera = nullptr;
	}
}

void UXRayComponentBase::HandleSkinColorChanged(const FLinearColor& NewSkinColor)
{
	OnSkinColorChanged(NewSkinColor);
}

void UXRayComponentBase::SetTrackingPaused(bool bPaused)
{
	if (bPaused == bTrackingPaused)
	{
		return;
	}
	bTrackingPaused = bPaused;

	if (bPaused)
	{
		if (bEffectActive)
		{
			SetComponentTickEnabled(false);
			TeardownEffect();
			bEffectActive = false;
			bAnyOccluding = false;
		}
		return;
	}

	// 한 번도 시작하지 못한 컴포넌트는 카메라조차 없으므로 처음부터 다시 탄다
	if (!Camera.IsValid())
	{
		TryStartTracking();
		return;
	}

	if (InitializeEffect())
	{
		bEffectActive = true;
		TimeSinceTrace = TraceInterval;
		SetComponentTickEnabled(true);
	}
}

void UXRayComponentBase::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeSinceTrace += DeltaTime;
	if (TimeSinceTrace >= TraceInterval)
	{
		TimeSinceTrace = 0.f;
		UpdateTrace();
	}

	UpdateEffect(DeltaTime);
}

void UXRayComponentBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		OwnerPawn->ReceiveControllerChangedDelegate.RemoveDynamic(this, &UXRayComponentBase::HandleControllerChanged);
	}

	if (ATromboneCharacterBase* OwnerCharacter = Cast<ATromboneCharacterBase>(GetOwner()))
	{
		OwnerCharacter->OnSkinColorChanged.RemoveDynamic(this, &UXRayComponentBase::HandleSkinColorChanged);
	}

	TeardownEffect();
	bEffectActive = false;
	Super::EndPlay(EndPlayReason);
}

void UXRayComponentBase::UpdateTrace()
{
	const AActor* Owner = GetOwner();
	if (!Camera.IsValid() || !Owner)
	{
		return;
	}

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(XRayOcclusion), false, Owner);

	TArray<FHitResult> Hits;
	GetWorld()->SweepMultiByObjectType(
		Hits,
		Camera->GetComponentLocation(),
		Owner->GetActorLocation(),
		FQuat::Identity,
		ObjectParams,
		FCollisionShape::MakeSphere(TraceSphereRadius),
		QueryParams);

	TArray<AActor*> Occluders;
	Occluders.Reserve(Hits.Num());
	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor)
		{
			continue;
		}
		if (bRequireOccluderTag && !HitActor->ActorHasTag(OccluderTag))
		{
			continue;
		}
		Occluders.AddUnique(HitActor);
	}

	bAnyOccluding = Occluders.Num() > 0;
	OnTraceUpdated(Occluders);
}

const APlayerController* UXRayComponentBase::GetOwningPlayerController() const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	return Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
}
