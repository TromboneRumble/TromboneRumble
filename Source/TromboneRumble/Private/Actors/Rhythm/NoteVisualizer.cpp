// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Rhythm/NoteVisualizer.h"
#include "Actors/Rhythm/RhythmActor.h"
#include "Subsystems/RhythmNoteChannelSubsystem.h"
#include "Subsystems/RhythmSubsystem.h"
#include "Subsystems/ActorPoolSubsystem.h"
#include "Components/StaticMeshComponents/RingHitBoxComponent.h"
#include "Kismet/GameplayStatics.h"

#if WITH_EDITOR
const FName ANoteVisualizer::ParamName_StartOuterRadius(TEXT("StartOuterRadius"));
const FName ANoteVisualizer::ParamName_StartInnerRadius(TEXT("StartInnerRadius"));
const FName ANoteVisualizer::ParamName_EndOuterRadius(TEXT("EndOuterRadius"));
const FName ANoteVisualizer::ParamName_EndInnerRadius(TEXT("EndInnerRadius"));
#endif

ANoteVisualizer::ANoteVisualizer()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	RingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RingMesh"));
	if (RingMesh)
	{
		SetRootComponent(RingMesh);
		RingMesh->SetCastShadow(false);
		RingMesh->bReceivesDecals = false;
		RingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		RingMesh->SetSimulatePhysics(false);
		RingMesh->SetGenerateOverlapEvents(false);
	}
	

}

void ANoteVisualizer::Init(const FNoteHandle& InNoteHandle, const EInstrumentType& InType, const EInstrumentType& HeldType)
{
	NoteHandle = InNoteHandle;
	InstrumentType = InType;
	if (InType == HeldType && EInstrumentType::Background < InstrumentType && InstrumentType < EInstrumentType::None && RingMesh)
	{
		ShowRing(true);
	}
	else
	{
		ShowRing(false);
	}
	const bool bNeedAttach =
		!CachedRingHitBoxComponent.IsValid()
		|| !GetRootComponent()
		|| GetRootComponent()->GetAttachParent() != CachedRingHitBoxComponent.Get();

	if (bNeedAttach)
	{
		FindPlayerCharacterAndAttach();
	}
#if WITH_EDITOR
	ValidateEndRadii();
#endif

	UnBindChannel();
	BindChannel();
}



void ANoteVisualizer::OnTakenFromPool_Implementation()
{
	IPoolable::OnTakenFromPool_Implementation();
	EnsureMID();
	SetAlpha(0.f);
}

void ANoteVisualizer::OnReturnToPool_Implementation()
{
	IPoolable::OnReturnToPool_Implementation();
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	CachedRingHitBoxComponent.Reset();
	ShowRing(false);
}

void ANoteVisualizer::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	EnsureMID();
	ApplyMaterialParams();
}

void ANoteVisualizer::BeginPlay()
{
	Super::BeginPlay();
	if (URhythmSubsystem* RhythmSubsystem = GetGameInstance()->GetSubsystem<URhythmSubsystem>())
	{
		RhythmSubsystem->OnInstrumentPicked.AddDynamic(this, &ThisClass::HandleOnInstrumentPicked);
	}
	EnsureMID();
	ApplyMaterialParams();
}

void ANoteVisualizer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnBindChannel();
	if (URhythmSubsystem* RhythmSubsystem = GetGameInstance()->GetSubsystem<URhythmSubsystem>())
	{
		RhythmSubsystem->OnInstrumentPicked.RemoveDynamic(this, &ThisClass::HandleOnInstrumentPicked);
	}
	Super::EndPlay(EndPlayReason);
}


void ANoteVisualizer::SetAlpha(float InAlpha)
{
	SizeAlpha = FMath::Clamp(InAlpha, 0.0f, MissEndAlpha);

	EnsureMID();
	if (MID)
	{
		MID->SetScalarParameterValue(ParamName_SizeAlpha, SizeAlpha);
	}
}

void ANoteVisualizer::ApplyMaterialParams()
{
	if (!MID) return;

	MID->SetScalarParameterValue(ParamName_SizeAlpha, SizeAlpha);
}

void ANoteVisualizer::BindChannel()
{
	if (URhythmNoteChannelSubsystem* RhythmNoteChannelSubsystem = GetWorld()->GetSubsystem<URhythmNoteChannelSubsystem>())
	{
		if (FNoteChannel* Channel = RhythmNoteChannelSubsystem->GetChannelById(NoteHandle.Id))
		{
			ProgressHandle = Channel->OnProgress.AddWeakLambda(this, [this](float Alpha)
				{
					SetAlpha(Alpha);
				});
			DespawnHandle = Channel->OnDespawn.AddWeakLambda(this, [this]()
				{
					ShowRing(false);
					if (UActorPoolSubsystem* Subsystem = GetWorld()->GetSubsystem<UActorPoolSubsystem>())
					{
						Subsystem->Release(this);
					}
				});
		}
	}
}

void ANoteVisualizer::UnBindChannel()
{
	if (URhythmNoteChannelSubsystem* RhythmNoteChannelSubsystem = GetWorld()->GetSubsystem<URhythmNoteChannelSubsystem>())
	{
		if (FNoteChannel* Channel = RhythmNoteChannelSubsystem->GetChannelById(NoteHandle.Id))
		{
			if (ProgressHandle.IsValid())
			{
				Channel->OnProgress.Remove(ProgressHandle);
				ProgressHandle.Reset();
			}
			if (DespawnHandle.IsValid())
			{
				Channel->OnDespawn.Remove(DespawnHandle);
				DespawnHandle.Reset();
			}
		}
	}
}

void ANoteVisualizer::ShowRing(bool bShow)
{
	if (!RingMesh) return;
	if (bShow)
	{
		RingMesh->SetHiddenInGame(false, true);
		RingMesh->SetVisibility(true, true);
	}
	else
	{
		RingMesh->SetHiddenInGame(true, true);
		RingMesh->SetVisibility(false, true);
	}

}

void ANoteVisualizer::FindPlayerCharacterAndAttach()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	APawn* Pawn = PC->GetPawn();
	if (!Pawn || !Pawn->IsLocallyControlled())
	{
		return;
	}

	URingHitBoxComponent* RingHitBox =
		Pawn->FindComponentByClass<URingHitBoxComponent>();

	if (!RingHitBox)
	{
		return;
	}
	
	AttachToComponent(
		RingHitBox,
		FAttachmentTransformRules::SnapToTargetIncludingScale
	);
	// 바닥의 히트박스 링과 같은 평면에서 깜빡이지 않게 띄우는 값
	SetActorRelativeLocation(FVector(0.0f, 0.0f, 2.0f));
	CachedRingHitBoxComponent = RingHitBox;
}

void ANoteVisualizer::HandleOnInstrumentPicked(EInstrumentType OldType, EInstrumentType NewType)
{
	if (!RingMesh) return;
	if (NewType == InstrumentType)
	{
		ShowRing(true);
	}
	else
	{
		ShowRing(false);
	}
}

void ANoteVisualizer::EnsureMID()
{
	if (!RingMesh) return;

	// 이미 만들어져 있으면 재사용
	if (MID) return;

	// 맵별 머티리얼이 지정되어 있으면 Element 0을 먼저 교체
	if (UMaterialInterface* OverrideMat = ResolvePerMapOverrideMaterial())
	{
		if (RingMesh->GetMaterial(0) != OverrideMat)
		{
			RingMesh->SetMaterial(0, OverrideMat);
		}
	}

	// Element 0에 이미 Material/MI가 들어있으면 Source Material 안 넣어도 됨
	MID = RingMesh->CreateDynamicMaterialInstance(0);
}

UMaterialInterface* ANoteVisualizer::ResolvePerMapOverrideMaterial() const
{
	const UWorld* World = GetWorld();
	if (!World || !World->GetGameInstance()) return nullptr;

	const URhythmSubsystem* RhythmSubsystem = World->GetGameInstance()->GetSubsystem<URhythmSubsystem>();
	if (!RhythmSubsystem) return nullptr;

	const ARhythmActor* CurrentRhythmActor = RhythmSubsystem->GetRegisteredRhythmActor(World);
	return CurrentRhythmActor ? CurrentRhythmActor->GetNoteVisualizerRingMaterial() : nullptr;
}

#if WITH_EDITOR
bool ANoteVisualizer::ComputeEndRadii(float StartOuter, float StartInner, float AnchorInner,
	float AnchorOuter, float& OutEndOuter, float& OutEndInner)
{
	if (AnchorOuter <= AnchorInner + KINDA_SMALL_NUMBER)
	{
		return false;
	}

	// 정타(알파 1.0)에 링 중심선이 히트박스 밴드 정중앙과 일치하게 맞춘다
	OutEndOuter = (AnchorInner + AnchorOuter) * 0.5f + (StartOuter - StartInner) * 0.5f;

	// 시작 링이 End보다 커야 줄어드는 그림이 된다
	if (OutEndOuter >= StartOuter - KINDA_SMALL_NUMBER)
	{
		return false;
	}

	// 두께가 알파에 선형이라, 시작 두께를 그대로 빼야 접근 내내 두께가 일정하다
	OutEndInner = FMath::Max(OutEndOuter - (StartOuter - StartInner), 0.0f);
	return true;
}

void ANoteVisualizer::ValidateEndRadii()
{
	// 반경 4종은 전부 MI 소유다. 여기선 저장값이 규칙과 맞는지 보기만 하고 고치지 않는다
	const URingHitBoxComponent* RingHitBox = CachedRingHitBoxComponent.Get();
	if (!RingHitBox) return;

	EnsureMID();
	if (!MID) return;

	auto ReadRadius = [this](const FName& ParamName, float Fallback)
		{
			float Value = Fallback;
			return MID->GetScalarParameterValue(ParamName, Value) ? Value : Fallback;
		};
	const float StartOuter = ReadRadius(ParamName_StartOuterRadius, RingHitBox->GetStartOuterRadius());

	// 같은 설정에서 노트마다 다시 찍지 않는다
	if (FMath::IsNearlyEqual(StartOuter, LastWarnedStartOuter, KINDA_SMALL_NUMBER)) return;

	const float StartInner = ReadRadius(ParamName_StartInnerRadius, RingHitBox->GetStartInnerRadius());
	const float EndOuter = ReadRadius(ParamName_EndOuterRadius, RingHitBox->GetEndOuterRadius());
	const float EndInner = ReadRadius(ParamName_EndInnerRadius, RingHitBox->GetEndInnerRadius());
	const float AnchorInner = RingHitBox->GetEndInnerRadius();
	const float AnchorOuter = RingHitBox->GetEndOuterRadius();

	float WantEndOuter = 0.0f, WantEndInner = 0.0f;
	if (!ComputeEndRadii(StartOuter, StartInner, AnchorInner, AnchorOuter, WantEndOuter, WantEndInner))
	{
		LastWarnedStartOuter = StartOuter;
		UE_LOG(LogTemp, Warning,
			TEXT("[NoteVisualizer] StartOuterRadius(%.5f)가 히트박스 밴드 중앙(%.5f)까지 줄어들 수 없어 End 반경 규칙을 세울 수 없다. ")
			TEXT("노트 MI의 StartOuterRadius를 더 크게 잡을 것"),
			StartOuter, (AnchorInner + AnchorOuter) * 0.5f);
		return;
	}

	if (WantEndInner <= 0.0f)
	{
		LastWarnedStartOuter = StartOuter;
		UE_LOG(LogTemp, Warning,
			TEXT("[NoteVisualizer] 링 두께(%.5f)가 판정선 크기(%.5f)보다 두꺼워 안쪽 반경이 0으로 잘린다. ")
			TEXT("판정선 근처에서 링이 꽉 찬 원이 된다"),
			StartOuter - StartInner, WantEndOuter);
		return;
	}

	constexpr float RadiusTolerance = 1e-4f;
	if (!FMath::IsNearlyEqual(EndOuter, WantEndOuter, RadiusTolerance)
		|| !FMath::IsNearlyEqual(EndInner, WantEndInner, RadiusTolerance))
	{
		LastWarnedStartOuter = StartOuter;
		UE_LOG(LogTemp, Warning,
			TEXT("[NoteVisualizer] 노트 MI에 저장된 End 반경이 규칙과 다르다. ")
			TEXT("EndOuterRadius %.5f -> %.5f, EndInnerRadius %.5f -> %.5f. ")
			TEXT("MI를 열고 StartOuterRadius를 한 번 건드리면 자동으로 맞춰진다"),
			EndOuter, WantEndOuter, EndInner, WantEndInner);
	}
}
#endif
