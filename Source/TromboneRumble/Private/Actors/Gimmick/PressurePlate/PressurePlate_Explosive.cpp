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

	ExplosionTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("ExplosionTimeline"));
}

void APressurePlate_Explosive::BeginPlay()
{
	Super::BeginPlay();
	if (EmissiveCurve)
	{
		FOnTimelineFloat ProgressFunction;
		ProgressFunction.BindUFunction(this, FName("UpdateEmissiveEffect"));
		ExplosionTimeline->AddInterpFloat(EmissiveCurve, ProgressFunction);

		FOnTimelineEvent FinishedFunction;
		FinishedFunction.BindUFunction(this, FName("TriggerExplosion"));
		ExplosionTimeline->SetTimelineFinishedFunc(FinishedFunction);
		ExplosionTimeline->SetLooping(true);
		ExplosionTimeline->SetTimelineLength(ExplosionTime);
	}
}

bool APressurePlate_Explosive::CanInteract_Implementation(AActor* InstigatorActor) const
{
	return InstigatorActor && InstigatorActor->IsA(APressurePlateBase::StaticClass()) && HasAuthority();
}

void APressurePlate_Explosive::Interact_Implementation(AActor* InstigatorActor)
{
	if (!HasAuthority()) return;

	SetActorLocation(InstigatorActor->GetActorLocation());

	if (APressurePlateBase* PressurePlate = Cast<APressurePlateBase>(InstigatorActor))
	{
		if (UStaticMeshComponent* PlateFrame = Cast<UStaticMeshComponent>(PressurePlate->GetDefaultSubobjectByName(TEXT("Frame"))))
		{
			DynamicMaterial = PlateFrame->CreateDynamicMaterialInstance(0);
		}
	}

	if (ExplosionTimeline)
	{
		ExplosionTimeline->PlayFromStart();
		FTimerHandle ExplosionTimer;
		GetWorld()->GetTimerManager().SetTimer(ExplosionTimer, this, &APressurePlate_Explosive::TriggerExplosion, ExplosionTime, false);
	}
}

void APressurePlate_Explosive::UpdateEmissiveEffect(float Value)
{
	if (DynamicMaterial)
	{
		DynamicMaterial->SetScalarParameterValue(TEXT("Emissive"), Value);
	}
}

void APressurePlate_Explosive::TriggerExplosion()
{
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	float FinalRadius = ExplosionSphere->GetScaledSphereRadius();
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(FinalRadius);

	// 거리 기반 감쇄 로직을 포함한 폭발 처리
	if (GetWorld()->OverlapMultiByChannel(OverlapResults, GetActorLocation(), FQuat::Identity, ECC_Pawn, SphereShape, Params))
	{
		for (const FOverlapResult& Overlap : OverlapResults)
		{
			AActor* HitActor = Overlap.GetActor();
			if (HitActor && HitActor->Implements<UCombatReceiver>())
			{
				FHitData HitData;

				FVector Dir = HitActor->GetActorLocation() - GetActorLocation();
				float Distance = Dir.Size();

				float DistanceAlpha = FMath::Clamp(1.0f - (Distance / FinalRadius), 0.1f, 1.0f);

				HitData.HitDirection = Dir.GetSafeNormal();
				HitData.KnockbackForce = MaxKnockbackForce * DistanceAlpha; // 거리에 따라 약해짐
				HitData.HitType = EHitReactionType::Ragdoll;

				ICombatReceiver::Execute_OnHitReceived(HitActor, HitData);
			}
		}
	}

	Destroy();
}

void APressurePlate_Explosive::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

