#include "Actors/Gimmick/PressurePlate/PressurePlate_Explosive.h"
#include "Actors/Gimmick/PressurePlate/PressurePlateBase.h"
#include "Components/SphereComponent.h"
#include "Interfaces/CombatReceiver.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "Kismet/KismetMathLibrary.h"

APressurePlate_Explosive::APressurePlate_Explosive()
{
	PrimaryActorTick.bCanEverTick = false;

	ExplosionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ExplosionSphere"));
	RootComponent = ExplosionSphere;

	EmissiveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("EmissiveTimeline"));
	ExpansionTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("ExpansionTimeline"));
}

void APressurePlate_Explosive::BeginPlay()
{
	Super::BeginPlay();

	if (ExplosionSphere)
	{
		MaxRadius = ExplosionSphere->GetUnscaledSphereRadius();
		ExplosionSphere->SetSphereRadius(1.0f);
	}

	if (EmissiveCurve)
	{
		FOnTimelineFloat Progress;
		Progress.BindUFunction(this, FName("UpdateEmissiveEffect"));
		EmissiveTimeline->AddInterpFloat(EmissiveCurve, Progress);
		EmissiveTimeline->SetLooping(true);
	}

	if (ExpansionCurve)
	{
		FOnTimelineFloat Progress;
		Progress.BindUFunction(this, FName("UpdateExplosionRadius"));
		ExpansionTimeline->AddInterpFloat(ExpansionCurve, Progress);

		// 팽창이 끝나면 지뢰 제거
		FOnTimelineEvent FinishedEvent;
		FinishedEvent.BindUFunction(this, FName("OnExpansionFinished"));
		ExpansionTimeline->SetTimelineFinishedFunc(FinishedEvent);
	}
}

bool APressurePlate_Explosive::CanInteract_Implementation(AActor* InstigatorActor) const
{
	return InstigatorActor && InstigatorActor->IsA(APressurePlateBase::StaticClass()) && HasAuthority();
}

void APressurePlate_Explosive::Interact_Implementation(AActor* InstigatorActor)
{
	if (!HasAuthority()) return;
	FVector NewLocation = InstigatorActor->GetActorLocation();
	NewLocation.Z -= 20.f;
	SetActorLocation(NewLocation);

	if (APressurePlateBase* PressurePlate = Cast<APressurePlateBase>(InstigatorActor))
	{
		if (UStaticMeshComponent* PlateFrame = Cast<UStaticMeshComponent>(PressurePlate->GetDefaultSubobjectByName(TEXT("Frame"))))
		{
			DynamicMaterial = PlateFrame->CreateDynamicMaterialInstance(0);
		}
	}

	//발판 발광 후 PreExplosionTime초 후 폭발
	if (EmissiveTimeline)
	{
		EmissiveTimeline->PlayFromStart();
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &APressurePlate_Explosive::StartExplosionExpansion, PreExplosionTime, false);
	}
}

void APressurePlate_Explosive::UpdateEmissiveEffect(float Value)
{
	if (DynamicMaterial)
	{
		DynamicMaterial->SetScalarParameterValue(TEXT("Emissive"), Value);
	}
}

void APressurePlate_Explosive::StartExplosionExpansion()
{
	EmissiveTimeline->Stop();

	TArray<FOverlapResult> OverlapResults;
	FCollisionShape DetectionSphere = FCollisionShape::MakeSphere(MaxRadius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->OverlapMultiByChannel(OverlapResults, GetActorLocation(), FQuat::Identity, ECC_Pawn, DetectionSphere, Params))
	{
		for (const FOverlapResult& Overlap : OverlapResults)
		{
			AActor* HitActor = Overlap.GetActor();
			if (HitActor && HitActor->Implements<UCombatReceiver>())
			{
				FHitData HitData;
				HitData.HitReaction = EHitReactionType::Ragdoll;
				HitData.HitInstigator = HitInstigatorType;
				ICombatReceiver::Execute_OnHitReceived(HitActor, HitData);
			}
		}
	}

	if (ExpansionTimeline)
	{
		ExpansionTimeline->PlayFromStart();
	}
}

void APressurePlate_Explosive::UpdateExplosionRadius(float Value)
{
	float NewRadius = FMath::Lerp(1.0f, MaxRadius, Value);
	ExplosionSphere->SetSphereRadius(NewRadius);
}

void APressurePlate_Explosive::OnExpansionFinished()
{
	if (!HasAuthority()) return;
	AActor* Spawner = GetOwner();

	if (Spawner && Spawner->IsA(APressurePlateBase::StaticClass()))
	{
		Spawner->Destroy();
	}

	Destroy();
}

void APressurePlate_Explosive::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

