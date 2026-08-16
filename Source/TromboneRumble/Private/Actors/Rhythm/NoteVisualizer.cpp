// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Rhythm/NoteVisualizer.h"
#include "Actors/Rhythm/RhythmActor.h"
#include "Subsystems/RhythmNoteChannelSubsystem.h"
#include "Subsystems/RhythmSubsystem.h"
#include "Subsystems/ActorPoolSubsystem.h"
#include "Components/StaticMeshComponents/RingHitBoxComponent.h"
#include "Kismet/GameplayStatics.h"

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
	// 이미 붙어있던 재사용 경로에서도 소멸 지점을 다시 계산해준다
	UpdateNestSizeAlpha();

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

	UpdateNestSizeAlpha();
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

void ANoteVisualizer::UpdateNestSizeAlpha()
{
	// 반경 4종은 전부 머티리얼/MI 소유다. 여기선 강제로 밀어넣지 않고 읽기만 한다
	// 히트박스는 "노트 외곽이 히트박스 안쪽에 맞물리면 소멸"의 앵커로만 쓰인다
	const URingHitBoxComponent* RingHitBox = CachedRingHitBoxComponent.Get();
	if (!RingHitBox) return;

	EnsureMID();
	if (!MID) return;

	auto ReadRadius = [this](const TCHAR* ParamName, float Fallback)
		{
			float Value = Fallback;
			return MID->GetScalarParameterValue(ParamName, Value) ? Value : Fallback;
		};
	const float StartOuter = ReadRadius(TEXT("StartOuterRadius"), RingHitBox->GetStartOuterRadius());
	const float StartInner = ReadRadius(TEXT("StartInnerRadius"), RingHitBox->GetStartInnerRadius());
	const float EndOuter = ReadRadius(TEXT("EndOuterRadius"), RingHitBox->GetEndOuterRadius());
	const float EndInner = ReadRadius(TEXT("EndInnerRadius"), RingHitBox->GetEndInnerRadius());
	const float AnchorInner = RingHitBox->GetEndInnerRadius();

	// 머티리얼이 Lerp(Start, End, SizeAlpha)라 1.0을 넘으면 반경이 계속 줄어든다.
	// 외곽이 히트박스 안쪽(AnchorInner)에 닿는 지점이 링이 히트박스를 벗어나는 크기다
	NestSizeAlpha = (StartOuter > EndOuter + KINDA_SMALL_NUMBER)
		? (StartOuter - AnchorInner) / (StartOuter - EndOuter)
		: 1.0f;

	// 안쪽 반경이 0을 뚫으면 링이 뒤집혀 보인다. 그 지점이 더 이르면 그쪽이 한계다
	if (StartInner > EndInner + KINDA_SMALL_NUMBER)
	{
		constexpr float MinRadius = 0.001f;
		NestSizeAlpha = FMath::Min(NestSizeAlpha, (StartInner - MinRadius) / (StartInner - EndInner));
	}

#if !UE_BUILD_SHIPPING
	// 벗어나는 지점과 소멸 지점이 어긋나면 링이 먼저 사라지거나 남은 채로 튄다
	constexpr float NestAlphaTolerance = 0.01f;
	if (!FMath::IsNearlyEqual(NestSizeAlpha, MissEndAlpha, NestAlphaTolerance)
		&& !FMath::IsNearlyEqual(NestSizeAlpha, LastWarnedNestAlpha, KINDA_SMALL_NUMBER))
	{
		LastWarnedNestAlpha = NestSizeAlpha;
		const float RecommendedEndOuter = StartOuter - (StartOuter - AnchorInner) / MissEndAlpha;
		UE_LOG(LogTemp, Warning,
			TEXT("[NoteVisualizer] 링이 히트박스를 벗어나는 지점(%.4f)이 소멸 지점(%.4f)과 다르다. ")
			TEXT("노트 머티리얼의 EndOuterRadius를 %.4f -> %.5f로 맞출 것"),
			NestSizeAlpha, MissEndAlpha, EndOuter, RecommendedEndOuter);
	}
#endif
}
