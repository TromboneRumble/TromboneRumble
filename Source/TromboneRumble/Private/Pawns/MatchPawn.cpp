#include "Pawns/MatchPawn.h"
#include "Components/CapsuleComponent.h"
#include "Components/ArrowComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/ActorComponents/NameplateComponent.h"
#include "Framework/DefaultPlayerState.h"
#include "Framework/GameState/MatchMenuGameState.h"

AMatchPawn::AMatchPawn()
{
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	CapsuleComponent->InitCapsuleSize(34.0f, 88.0f);
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CapsuleComponent->SetCollisionObjectType(ECC_Pawn);
	CapsuleComponent->SetCollisionResponseToAllChannels(ECR_Block);
	CapsuleComponent->SetShouldUpdatePhysicsVolume(true);
	RootComponent = CapsuleComponent;
	
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	SkeletalMeshComponent->SetupAttachment(RootComponent);
	SkeletalMeshComponent->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -90.0f), FRotator(0.0f, -90.0f, 0.0f));
	
	NameplateComponent = CreateDefaultSubobject<UNameplateComponent>(TEXT("NameplateComponent"));

#if WITH_EDITORONLY_DATA
	ArrowComponent = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	if (ArrowComponent)
	{
		ArrowComponent->ArrowColor = FColor(150, 200, 255);
		ArrowComponent->bIsScreenSizeScaled = true;
		ArrowComponent->SetupAttachment(CapsuleComponent);
	}
#endif

	// Disable movement replication by default because we most likely want to relocate lobby pawns after spawning.
	AActor::SetReplicateMovement(false);
}

void AMatchPawn::UpdateSkinFromPlayerState() const
{
	if (const ADefaultPlayerState* DPS = GetPlayerState<ADefaultPlayerState>())
	{
		const FLinearColor SkinColor = DPS->GetSkinColor();
		if (SkinMID)
		{
			SkinMID->SetVectorParameterValue(TEXT("BaseColor"), SkinColor);
		}
		if (FaceMID)
		{
			FaceMID->SetVectorParameterValue(TEXT("BaseColor"), SkinColor);
		}
	}
}

void AMatchPawn::BeginPlay()
{
	Super::BeginPlay();

	if (AMatchMenuGameState* GameState = GetWorld()->GetGameState<AMatchMenuGameState>())
	{
		GameState->HandleLobbyPawnCreated(this);
	}
	
	if (UMaterialInterface* CurrentSkinMat = SkeletalMeshComponent->GetMaterial(SkinMaterialIndex))
	{
		SkinMID = Cast<UMaterialInstanceDynamic>(CurrentSkinMat);
		if (!SkinMID)
		{
			SkinMID = SkeletalMeshComponent->CreateAndSetMaterialInstanceDynamic(SkinMaterialIndex);
		}
	}
	if (UMaterialInterface* CurrentFaceMat = SkeletalMeshComponent->GetMaterial(FaceMaterialIndex))
	{
		FaceMID = Cast<UMaterialInstanceDynamic>(CurrentFaceMat);
		if (!FaceMID)
		{
			FaceMID = SkeletalMeshComponent->CreateAndSetMaterialInstanceDynamic(FaceMaterialIndex);
		}
	}
	
	// Apply a skin color in server side
	UpdateSkinFromPlayerState();
}

void AMatchPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AMatchMenuGameState* GameState = GetWorld()->GetGameState<AMatchMenuGameState>())
	{
		GameState->HandleLobbyPawnPreDestroyed(this);
	}

	Super::EndPlay(EndPlayReason);
}

void AMatchPawn::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Apply a skin color in client side
	UpdateSkinFromPlayerState();
}
